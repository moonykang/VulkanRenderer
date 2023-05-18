#include "scene/basicScene.h"
#include "render/rendergraph.h"
#include "render/renderpass.h"
#include "rhi/context.h"
#include "rhi/rendertarget.h"
#include "rhi/pipeline.h"
#include "rhi/accelerationStructure.h"
#include "model/object.h"
#include <algorithm>

namespace scene
{
BasicScene::BasicScene()
{
	enableRayTracing = true;
	enableScratchBuffer = enableRayTracing;
}

render::Renderpass* BasicScene::initSurfaceRenderpass(rhi::Context* context, platform::AssetManager* assetManager, rhi::Texture* inputRenderTarget)
{
	auto renderpass = renderGraph->allocateRenderpass("Present", rhi::RenderTargetType::Surface);
	renderpass->addBeginTransition(inputRenderTarget, rhi::MemoryAccess::Read);
	auto renderTarget = renderpass->initRenderTarget(context, context->getWidth(), context->getHeight());

	rhi::Attachment* attachment = new rhi::Attachment(nullptr, rhi::AttachmentOp::Clear, rhi::AttachmentOp::Store, { 0.f, 0.f, 0.f, 1.f });
	renderTarget->addColorAttachment(attachment);

	rhi::Subpass* subpass = new rhi::Subpass();
	subpass->addColorAttachment(0);
	renderTarget->addSubpass(subpass);

	model::Object* object = renderpass->generateObject(context);

	object->loadPredefinedScreen(context);
	object->updateShaderCode(assetManager, rhi::ShaderStage::Vertex, "shaders/screen.vert.spv");
	object->updateShaderCode(assetManager, rhi::ShaderStage::Fragment, "shaders/screen.frag.spv");
	
	object->registerDescriptor(rhi::DescriptorType::Uniform_Buffer, rhi::ShaderStage::Vertex, sceneUniformBuffer);
	object->registerDescriptor(rhi::DescriptorType::Combined_Image_Sampler, rhi::ShaderStage::Fragment, inputRenderTarget);
	object->instantiate(context, glm::mat4(1.f));

	return renderpass;
}

void BasicScene::initSceneRenderGraph(rhi::Context* context, platform::AssetManager* assetManager)
{
    if (!enableRayTracing)
    {
        sceneLight.flipY = true;
    }

	// TODO
	uint32_t width = context->getWidth();
	uint32_t height = context->getHeight();

	rhi::Texture* shadowMapTexture = nullptr;
	model::Object* sceneObject = nullptr;

	if (!enableRayTracing)
	{
		uint32_t shadowMapWidth = 2048;
		uint32_t shadowMapHeight = 2048;
			// Depth
		shadowMapTexture = allocateSceneTexture(context, rhi::Format::D32_FLOAT_S8X24_UINT, shadowMapWidth, shadowMapHeight, rhi::ImageLayout::DepthStencilAttachment, rhi::ImageUsage::DEPTH_STENCIL_ATTACHMENT | rhi::ImageUsage::SAMPLED);

		auto renderpass = renderGraph->allocateRenderpass("ShadowMap", rhi::RenderTargetType::Graphics);
		renderpass->addBeginTransition(shadowMapTexture, rhi::MemoryAccess::Write);

		auto renderTarget = renderpass->initRenderTarget(context, shadowMapWidth, shadowMapHeight);
		rhi::Attachment* attachmentDS = new rhi::Attachment(shadowMapTexture, rhi::AttachmentOp::Clear, rhi::AttachmentOp::Store, rhi::AttachmentOp::Clear, rhi::AttachmentOp::Store, { 1.f, 0.f, 0.f, 0.f });
		uint32_t indexDepthStencil = renderTarget->addDepthAttachment(attachmentDS);

		rhi::Subpass* subpass = new rhi::Subpass();
		subpass->setDepthStencilAttachment(indexDepthStencil);

		renderTarget->addSubpass(subpass);

		{
			sceneObject = renderpass->generateObject(context);
			auto pipelineState = sceneObject->getPipelineState();
			pipelineState->colorBlendMasks.clear();
			pipelineState->depthStencilState = rhi::PipelineState::DepthStencilState(true, true, rhi::CompareOp::LESS_OR_EQUAL);

			sceneObject->loadGltfModel(context, assetManager, "models/sponza/", "sponza.gltf"
				//, model::GltfLoadingFlag::FlipY | model::GltfLoadingFlag::PreTransformVertices
				, model::GltfLoadingFlag::PreTransformVertices
				, rhi::VertexChannel::Position | rhi::VertexChannel::Uv | rhi::VertexChannel::Normal
				| rhi::VertexChannel::Tangent | rhi::VertexChannel::Bitangent, rhi::MaterialFlag::BaseColorTexture);

			sceneObject->updateShaderCode(assetManager, rhi::ShaderStage::Vertex, "shaders/shadowmap.vert.spv");
			sceneObject->registerDescriptor(rhi::DescriptorType::Uniform_Buffer, rhi::ShaderStage::Vertex | rhi::ShaderStage::Fragment, sceneUniformBuffer);

			sceneObject->instantiate(context, glm::mat4(1.f));
		}
	}

	// RGB: Albedo, A: Metallic
	rhi::Texture* gBufferA = allocateSceneTexture(context, rhi::Format::R8G8B8A8_UNORM, width, height, rhi::ImageLayout::ColorAttachment, rhi::ImageUsage::COLOR_ATTACHMENT | rhi::ImageUsage::SAMPLED);
	// RG: Normal, BA: Motion Vector
	rhi::Texture* gBufferB = allocateSceneTexture(context, rhi::Format::R16G16B16A16_FLOAT, width, height, rhi::ImageLayout::ColorAttachment, rhi::ImageUsage::COLOR_ATTACHMENT | rhi::ImageUsage::SAMPLED);
	// R: Roughness, G: Curvature, B: Mesh ID, A: Linear Z
	rhi::Texture* gBufferC = allocateSceneTexture(context, rhi::Format::R16G16B16A16_FLOAT, width, height, rhi::ImageLayout::ColorAttachment, rhi::ImageUsage::COLOR_ATTACHMENT | rhi::ImageUsage::SAMPLED);
	// Depth
	rhi::Texture* sceneDepth = allocateSceneTexture(context, rhi::Format::D32_FLOAT_S8X24_UINT, width, height, rhi::ImageLayout::DepthStencilAttachment, rhi::ImageUsage::DEPTH_STENCIL_ATTACHMENT | rhi::ImageUsage::SAMPLED);

	rhi::Texture* sceneColor = allocateSceneTexture(context, rhi::Format::R8G8B8A8_UNORM, width, height, rhi::ImageLayout::ColorAttachment, rhi::ImageUsage::COLOR_ATTACHMENT | rhi::ImageUsage::SAMPLED);

	{
		render::GraphicsRenderpass* renderpass = reinterpret_cast<render::GraphicsRenderpass*>(renderGraph->allocateRenderpass("GBuffer", rhi::RenderTargetType::Graphics));
		renderpass->addBeginTransition(gBufferA, rhi::MemoryAccess::Write);
		renderpass->addBeginTransition(gBufferB, rhi::MemoryAccess::Write);
		renderpass->addBeginTransition(gBufferC, rhi::MemoryAccess::Write);
		renderpass->addBeginTransition(sceneDepth, rhi::MemoryAccess::Write);

		auto renderTarget = renderpass->initRenderTarget(context, width, height);

		rhi::Attachment* attachmentA = new rhi::Attachment(gBufferA, rhi::AttachmentOp::Clear, rhi::AttachmentOp::Store, { 0.f, 0.f, 0.f, 1.f });
		rhi::Attachment* attachmentB = new rhi::Attachment(gBufferB, rhi::AttachmentOp::Clear, rhi::AttachmentOp::Store, { 0.f, 0.f, 0.f, 1.f });
		rhi::Attachment* attachmentC = new rhi::Attachment(gBufferC, rhi::AttachmentOp::Clear, rhi::AttachmentOp::Store, { 0.f, 0.f, 0.f, 1.f });
		rhi::Attachment* attachmentDS = new rhi::Attachment(sceneDepth, rhi::AttachmentOp::Clear, rhi::AttachmentOp::Store, rhi::AttachmentOp::Clear, rhi::AttachmentOp::Store, { 1.f, 0.f, 0.f, 0.f });

		uint32_t indexGBufferA = renderTarget->addColorAttachment(attachmentA);
		uint32_t indexGBufferB = renderTarget->addColorAttachment(attachmentB);
		uint32_t indexGBufferC = renderTarget->addColorAttachment(attachmentC);
		uint32_t indexDepthStencil = renderTarget->addDepthAttachment(attachmentDS);

		rhi::Subpass* subpass = new rhi::Subpass();
		subpass->addColorAttachment(indexGBufferA);
		subpass->addColorAttachment(indexGBufferB);
		subpass->addColorAttachment(indexGBufferC);
		subpass->setDepthStencilAttachment(indexDepthStencil);

		renderTarget->addSubpass(subpass);

		{
			model::Object* object = renderpass->generateObject(context);
			object->loadGltfModel(context, assetManager, "models/sponza/", "sponza.gltf"
				//, model::GltfLoadingFlag::FlipY | model::GltfLoadingFlag::PreTransformVertices
				, model::GltfLoadingFlag::PreTransformVertices
				, rhi::VertexChannel::Position | rhi::VertexChannel::Uv | rhi::VertexChannel::Normal
				| rhi::VertexChannel::Tangent | rhi::VertexChannel::Bitangent, rhi::MaterialFlag::BaseColorTexture);

			auto pipelineState = object->getPipelineState();
			pipelineState->colorBlendMasks.clear();
			pipelineState->colorBlendMasks.push_back(rhi::ColorBlendMask::COLOR_COMPONENT_ALL_BIT);
			pipelineState->colorBlendMasks.push_back(rhi::ColorBlendMask::COLOR_COMPONENT_ALL_BIT);
			pipelineState->colorBlendMasks.push_back(rhi::ColorBlendMask::COLOR_COMPONENT_ALL_BIT);
			pipelineState->depthStencilState = rhi::PipelineState::DepthStencilState(true, true, rhi::CompareOp::LESS_OR_EQUAL);

			object->updateShaderCode(assetManager, rhi::ShaderStage::Vertex, "shaders/gbuffer.vert.spv");
			object->updateShaderCode(assetManager, rhi::ShaderStage::Fragment, "shaders/gbuffer.frag.spv");
			object->registerDescriptor(rhi::DescriptorType::Uniform_Buffer, rhi::ShaderStage::Vertex | rhi::ShaderStage::Fragment, sceneUniformBuffer);

			object->instantiate(context, glm::mat4(1.f));

			registerObject(context, object);
		}
	}

	float scale = 0.5f;
    sceneUniformBufferObject.inverse_scale = std::ceil(1.0f / scale);

	uint32_t rayShadowWidth = width * scale;
	uint32_t rayShadowHeight = height * scale;
	uint32_t numThreadX = 8;
	uint32_t numTHreadY = 4;
	uint32_t computeWidth = std::ceil(float(rayShadowWidth) / numThreadX);
	uint32_t computeHeight = std::ceil(float(rayShadowHeight) / numTHreadY);

	rhi::Texture* shadowRayqueryTarget = allocateSceneTexture(context, rhi::Format::R32_UINT, computeWidth, computeHeight, rhi::ImageLayout::ComputeShaderWrite, rhi::ImageUsage::STORAGE | rhi::ImageUsage::SAMPLED);
	rhi::Texture* blueNoiseSobolTexture = allocateSceneTexture(context, rhi::Format::R8G8B8A8_UNORM, width, height, rhi::ImageLayout::ComputeShaderReadOnly, rhi::ImageUsage::SAMPLED | rhi::ImageUsage::TRANSFER_DST);
	blueNoiseSobolTexture->loadTexture(assetManager, "textures/blue_noise/sobol_256_4d.png");
	rhi::Texture* blueNoiseScrambleTexture = allocateSceneTexture(context, rhi::Format::R8G8B8A8_UNORM, width, height, rhi::ImageLayout::ComputeShaderReadOnly, rhi::ImageUsage::SAMPLED | rhi::ImageUsage::TRANSFER_DST);
	blueNoiseScrambleTexture->loadTexture(assetManager, "textures/blue_noise/scrambling_ranking_128x128_2d_1spp.png");

	if (enableRayTracing)
	{
		auto renderpass = renderGraph->allocateRenderpass("RayTracingShadow", rhi::RenderTargetType::Compute);
		renderpass->addBeginTransition(shadowRayqueryTarget, rhi::MemoryAccess::General);
		renderpass->addBeginTransition(gBufferA, rhi::MemoryAccess::Read);
		renderpass->addBeginTransition(gBufferB, rhi::MemoryAccess::Read);
		renderpass->addBeginTransition(gBufferC, rhi::MemoryAccess::Read);
		renderpass->addBeginTransition(sceneDepth, rhi::MemoryAccess::Read);
		renderpass->addBeginTransition(blueNoiseSobolTexture, rhi::MemoryAccess::Read);
		renderpass->addBeginTransition(blueNoiseScrambleTexture, rhi::MemoryAccess::Read);

		renderpass->initRenderTarget(context, computeWidth, computeHeight);

		model::ComputeObject* object = reinterpret_cast<model::ComputeObject*>(renderpass->generateObject(context));
		object->updateShaderCode(assetManager, rhi::ShaderStage::Compute, "shaders/shadows/shadowsRayQuery.comp.spv");
		object->registerDescriptor(rhi::DescriptorType::Uniform_Buffer, rhi::ShaderStage::Compute, sceneUniformBuffer);
		object->registerDescriptor(rhi::DescriptorType::Acceleration_structure, rhi::ShaderStage::Compute, accStructureManager);
		object->registerDescriptor(rhi::DescriptorType::Combined_Image_Sampler, rhi::ShaderStage::Compute, gBufferA);
		object->registerDescriptor(rhi::DescriptorType::Combined_Image_Sampler, rhi::ShaderStage::Compute, gBufferB);
		object->registerDescriptor(rhi::DescriptorType::Combined_Image_Sampler, rhi::ShaderStage::Compute, gBufferC);
		object->registerDescriptor(rhi::DescriptorType::Combined_Image_Sampler, rhi::ShaderStage::Compute, sceneDepth);
		object->registerDescriptor(rhi::DescriptorType::Storage_Image, rhi::ShaderStage::Compute, shadowRayqueryTarget);

		object->registerDescriptor(rhi::DescriptorType::Combined_Image_Sampler, rhi::ShaderStage::Compute, blueNoiseSobolTexture);
		object->registerDescriptor(rhi::DescriptorType::Combined_Image_Sampler, rhi::ShaderStage::Compute, blueNoiseScrambleTexture);
		// TODO
		object->setGroupCount(computeWidth, computeHeight, 1);
	}

	// Temporal acculmulation
	rhi::Texture* temporalAccumulationTarget = allocateSceneTexture(context, rhi::Format::R16G16_FLOAT, 
		rayShadowWidth, rayShadowHeight, rhi::ImageLayout::ComputeShaderWrite, 
		rhi::ImageUsage::STORAGE | rhi::ImageUsage::SAMPLED | rhi::ImageUsage::TRANSFER_DST | rhi::ImageUsage::TRANSFER_SRC);

	rhi::Texture* temporalMomentsTarget = allocateSceneTexture(context, rhi::Format::R16G16B16A16_FLOAT,
		rayShadowWidth, rayShadowHeight, rhi::ImageLayout::ComputeShaderWrite,
		rhi::ImageUsage::STORAGE | rhi::ImageUsage::SAMPLED | rhi::ImageUsage::TRANSFER_DST | rhi::ImageUsage::TRANSFER_SRC);

	rhi::StorageBuffer* denoiseTileCoordsBuffer = allocateSceneStorageBuffer(context, 
		sizeof(glm::ivec2) * computeWidth * computeHeight, rhi::BufferUsage::BUFFER_STORAGE_BUFFER);
	rhi::StorageBuffer* denoiseDispatchArgsBuffer = allocateSceneStorageBuffer(context,
		sizeof(int32_t) * 3, rhi::BufferUsage::BUFFER_STORAGE_BUFFER | rhi::BufferUsage::BUFFER_INDIRECT_BUFFER);
	rhi::StorageBuffer* shadowTileCoordsBuffer = allocateSceneStorageBuffer(context,
		sizeof(glm::ivec2) * computeWidth * computeHeight, rhi::BufferUsage::BUFFER_STORAGE_BUFFER);
	rhi::StorageBuffer* shadowDispatchArgsBuffer = allocateSceneStorageBuffer(context,
		sizeof(int32_t) * 3, rhi::BufferUsage::BUFFER_STORAGE_BUFFER | rhi::BufferUsage::BUFFER_INDIRECT_BUFFER);

	if (enableRayTracing)
	{
		// reset args
		{
			auto renderpass = renderGraph->allocateRenderpass("Reset args", rhi::RenderTargetType::Compute);
			renderpass->addBeginTransition(denoiseTileCoordsBuffer, rhi::MemoryAccess::Write);
			renderpass->addBeginTransition(denoiseDispatchArgsBuffer, rhi::MemoryAccess::Write);
			renderpass->addBeginTransition(shadowTileCoordsBuffer, rhi::MemoryAccess::Write);
			renderpass->addBeginTransition(shadowDispatchArgsBuffer, rhi::MemoryAccess::Write);

			renderpass->initRenderTarget(context, computeWidth, computeHeight);

			model::ComputeObject* object = reinterpret_cast<model::ComputeObject*>(renderpass->generateObject(context));
			object->updateShaderCode(assetManager, rhi::ShaderStage::Compute, "shaders/shadows/shadows_denoise_reset_args.comp.spv");
			object->registerDescriptor(rhi::DescriptorType::Storage_Buffer, rhi::ShaderStage::Compute, denoiseTileCoordsBuffer);
			object->registerDescriptor(rhi::DescriptorType::Storage_Buffer, rhi::ShaderStage::Compute, denoiseDispatchArgsBuffer);
			object->registerDescriptor(rhi::DescriptorType::Storage_Buffer, rhi::ShaderStage::Compute, shadowTileCoordsBuffer);
			object->registerDescriptor(rhi::DescriptorType::Storage_Buffer, rhi::ShaderStage::Compute, shadowDispatchArgsBuffer);

			object->setGroupCount(1, 1, 1);
		}

		// unpack
		{
			const int NUM_THREADS_X = 8;
			const int NUM_THREADS_Y = 8;

			auto renderpass = renderGraph->allocateRenderpass("Shadow unpack", rhi::RenderTargetType::Compute);
			renderpass->addBeginTransition(shadowRayqueryTarget, rhi::MemoryAccess::Read);
			renderpass->addBeginTransition(sceneDepth, rhi::MemoryAccess::Read);
			renderpass->addBeginTransition(temporalAccumulationTarget, rhi::MemoryAccess::General);
			renderpass->addBeginTransition(temporalMomentsTarget, rhi::MemoryAccess::General);

			renderpass->addBeginTransition(denoiseTileCoordsBuffer, rhi::MemoryAccess::Write);
			renderpass->addBeginTransition(denoiseDispatchArgsBuffer, rhi::MemoryAccess::Write);
			renderpass->addBeginTransition(shadowTileCoordsBuffer, rhi::MemoryAccess::Write);
			renderpass->addBeginTransition(shadowDispatchArgsBuffer, rhi::MemoryAccess::Write);

			renderpass->initRenderTarget(context, computeWidth, computeHeight);

			model::ComputeObject* object = reinterpret_cast<model::ComputeObject*>(renderpass->generateObject(context));
			object->updateShaderCode(assetManager, rhi::ShaderStage::Compute, "shaders/shadows/shadows_unpack.comp.spv");
            object->registerDescriptor(rhi::DescriptorType::Uniform_Buffer, rhi::ShaderStage::Compute, sceneUniformBuffer);
			object->registerDescriptor(rhi::DescriptorType::Combined_Image_Sampler, rhi::ShaderStage::Compute, shadowRayqueryTarget);
			object->registerDescriptor(rhi::DescriptorType::Storage_Image, rhi::ShaderStage::Compute, temporalAccumulationTarget);
			object->registerDescriptor(rhi::DescriptorType::Storage_Image, rhi::ShaderStage::Compute, temporalMomentsTarget);
			object->registerDescriptor(rhi::DescriptorType::Combined_Image_Sampler, rhi::ShaderStage::Compute, sceneDepth);

			object->registerDescriptor(rhi::DescriptorType::Storage_Buffer, rhi::ShaderStage::Compute, denoiseTileCoordsBuffer);
			object->registerDescriptor(rhi::DescriptorType::Storage_Buffer, rhi::ShaderStage::Compute, denoiseDispatchArgsBuffer);
			object->registerDescriptor(rhi::DescriptorType::Storage_Buffer, rhi::ShaderStage::Compute, shadowTileCoordsBuffer);
			object->registerDescriptor(rhi::DescriptorType::Storage_Buffer, rhi::ShaderStage::Compute, shadowDispatchArgsBuffer);

			object->setGroupCount(std::ceil((float)rayShadowWidth / NUM_THREADS_X), std::ceil((float)rayShadowHeight / NUM_THREADS_Y), 1);

			renderpass->addEndTransition(temporalAccumulationTarget, rhi::MemoryAccess::Read);
			renderpass->addEndTransition(denoiseTileCoordsBuffer, rhi::MemoryAccess::Read);
			renderpass->addEndTransition(denoiseDispatchArgsBuffer, rhi::MemoryAccess::Read | rhi::MemoryAccess::Indirect);
			renderpass->addEndTransition(shadowTileCoordsBuffer, rhi::MemoryAccess::Read);
			renderpass->addEndTransition(shadowDispatchArgsBuffer, rhi::MemoryAccess::Read | rhi::MemoryAccess::Indirect);
		}

		shadowMapTexture = temporalAccumulationTarget;
	}

	bool bFilter = false;
	if (bFilter && enableRayTracing)
	{
		uint32_t filter_iterations = 4;
		// a trous filter
		rhi::Texture* aTrousFilterTarget1 = allocateSceneTexture(context, rhi::Format::R16G16_FLOAT,
			rayShadowWidth, rayShadowHeight, rhi::ImageLayout::FragmentShaderReadOnly,
			rhi::ImageUsage::STORAGE | rhi::ImageUsage::SAMPLED | rhi::ImageUsage::TRANSFER_DST | rhi::ImageUsage::TRANSFER_SRC);
		rhi::Texture* aTrousFilterTarget2 = allocateSceneTexture(context, rhi::Format::R16G16_FLOAT,
			rayShadowWidth, rayShadowHeight, rhi::ImageLayout::ComputeShaderWrite,
			rhi::ImageUsage::STORAGE | rhi::ImageUsage::SAMPLED | rhi::ImageUsage::TRANSFER_DST | rhi::ImageUsage::TRANSFER_SRC);

		bool ping_pong = false;
		std::vector<rhi::Texture*> aTrousFilterTarget =
		{
			aTrousFilterTarget1,
			aTrousFilterTarget2
		};

		int32_t read_idx = 0;
		int32_t write_idx = 1;

		for (int i = 0; i < filter_iterations; i++)
		{
			auto renderpass = renderGraph->allocateRenderpass("A trous filter iterations " + ('0' + (char)i), rhi::RenderTargetType::Compute);
			renderpass->initRenderTarget(context, rayShadowWidth, rayShadowHeight);

			if (i != 0)
			{
				renderpass->addBeginTransition(aTrousFilterTarget[read_idx], rhi::MemoryAccess::Read);
			}
			renderpass->addBeginTransition(aTrousFilterTarget[write_idx], rhi::MemoryAccess::General);

			renderpass->addClearColorTexture(aTrousFilterTarget[write_idx], { 1.f, 1.f, 1.f, 1.f });

			// copy shadows
			{
				model::ComputeObject* object = reinterpret_cast<model::ComputeObject*>(renderpass->generateObject(context));
				object->updateShaderCode(assetManager, rhi::ShaderStage::Compute, "shaders/shadows/shadows_denoise_copy_shadow_tiles.comp.spv");
				object->registerDescriptor(rhi::DescriptorType::Storage_Image, rhi::ShaderStage::Compute, aTrousFilterTarget[write_idx]);
				object->registerDescriptor(rhi::DescriptorType::Storage_Buffer, rhi::ShaderStage::Compute, shadowTileCoordsBuffer);
				object->setIndirect(shadowDispatchArgsBuffer);
			}

			bool doFilter = true;
			// filter

			if (doFilter)
			{
				model::ComputeObject* object = reinterpret_cast<model::ComputeObject*>(renderpass->generateObject(context));
				object->updateShaderCode(assetManager, rhi::ShaderStage::Compute, "shaders/shadows/shadows_denoise_atrous.comp.spv");
				object->registerDescriptor(rhi::DescriptorType::Storage_Image, rhi::ShaderStage::Compute, aTrousFilterTarget[write_idx]);

				if (i == 0)
				{
					object->registerDescriptor(rhi::DescriptorType::Combined_Image_Sampler, rhi::ShaderStage::Compute, temporalAccumulationTarget);
				}
				else
				{
					object->registerDescriptor(rhi::DescriptorType::Combined_Image_Sampler, rhi::ShaderStage::Compute, aTrousFilterTarget[read_idx]);
				}

				object->registerDescriptor(rhi::DescriptorType::Combined_Image_Sampler, rhi::ShaderStage::Compute, gBufferA);
				object->registerDescriptor(rhi::DescriptorType::Combined_Image_Sampler, rhi::ShaderStage::Compute, gBufferB);
				object->registerDescriptor(rhi::DescriptorType::Combined_Image_Sampler, rhi::ShaderStage::Compute, gBufferC);
				object->registerDescriptor(rhi::DescriptorType::Combined_Image_Sampler, rhi::ShaderStage::Compute, sceneDepth);
				object->registerDescriptor(rhi::DescriptorType::Storage_Buffer, rhi::ShaderStage::Compute, denoiseTileCoordsBuffer);
				object->setIndirect(denoiseDispatchArgsBuffer);
			}
			shadowMapTexture = aTrousFilterTarget[write_idx];
			std::swap(read_idx, write_idx);
		}
	}
	
	rhi::Texture* upSampleTarget = allocateSceneTexture(context, rhi::Format::R16_FLOAT,
		width, height, rhi::ImageLayout::ComputeShaderWrite,
		rhi::ImageUsage::STORAGE | rhi::ImageUsage::SAMPLED | rhi::ImageUsage::TRANSFER_DST);

	if (enableRayTracing && scale < 1.0f)
	{
		const int NUM_THREADS_X = 32;
		const int NUM_THREADS_Y = 32;
		rhi::Texture* upSampleInput = shadowMapTexture;

		auto renderpass = renderGraph->allocateRenderpass("Upsample", rhi::RenderTargetType::Compute);
		renderpass->addBeginTransition(upSampleTarget, rhi::MemoryAccess::General);
		renderpass->addBeginTransition(upSampleInput, rhi::MemoryAccess::Read);

		renderpass->initRenderTarget(context, width, height);

		model::ComputeObject* object = reinterpret_cast<model::ComputeObject*>(renderpass->generateObject(context));
		object->updateShaderCode(assetManager, rhi::ShaderStage::Compute, "shaders/shadows/shadows_upsample.comp.spv");
        object->registerDescriptor(rhi::DescriptorType::Uniform_Buffer, rhi::ShaderStage::Compute, sceneUniformBuffer);
		object->registerDescriptor(rhi::DescriptorType::Storage_Image, rhi::ShaderStage::Compute, upSampleTarget);
		object->registerDescriptor(rhi::DescriptorType::Combined_Image_Sampler, rhi::ShaderStage::Compute, upSampleInput);
		object->registerDescriptor(rhi::DescriptorType::Combined_Image_Sampler, rhi::ShaderStage::Compute, gBufferA);
		object->registerDescriptor(rhi::DescriptorType::Combined_Image_Sampler, rhi::ShaderStage::Compute, gBufferB);
		object->registerDescriptor(rhi::DescriptorType::Combined_Image_Sampler, rhi::ShaderStage::Compute, gBufferC);
		object->registerDescriptor(rhi::DescriptorType::Combined_Image_Sampler, rhi::ShaderStage::Compute, sceneDepth);

		object->setGroupCount(std::ceil((float)width / NUM_THREADS_X), std::ceil((float)height / NUM_THREADS_Y), 1);

		shadowMapTexture = upSampleTarget;
	}

	// deferred pass
	{
		auto renderpass = renderGraph->allocateRenderpass("Deferred", rhi::RenderTargetType::Graphics);
		renderpass->addBeginTransition(gBufferA, rhi::MemoryAccess::Read);
		renderpass->addBeginTransition(gBufferB, rhi::MemoryAccess::Read);
		renderpass->addBeginTransition(gBufferC, rhi::MemoryAccess::Read);
		renderpass->addBeginTransition(sceneDepth, rhi::MemoryAccess::Read);
		renderpass->addBeginTransition(sceneColor, rhi::MemoryAccess::Write);
		renderpass->addBeginTransition(shadowMapTexture, rhi::MemoryAccess::Read);

		auto renderTarget = renderpass->initRenderTarget(context, width, height);

		rhi::Attachment* attachment = new rhi::Attachment(sceneColor, rhi::AttachmentOp::Clear, rhi::AttachmentOp::Store, { 0.f, 0.f, 0.f, 1.f });
		uint32_t indexSceneColorAttachment = renderTarget->addColorAttachment(attachment);

		rhi::Subpass* subpass = new rhi::Subpass();
		subpass->addColorAttachment(indexSceneColorAttachment);
		renderTarget->addSubpass(subpass);

		{
			model::Object* object = renderpass->generateObject(context);
			object->loadPredefinedScreen(context);
			object->updateShaderCode(assetManager, rhi::ShaderStage::Vertex, "shaders/screen.vert.spv");
			if (enableRayTracing)
			{
				object->updateShaderCode(assetManager, rhi::ShaderStage::Fragment, "shaders/deferred_rayshadows.frag.spv");
			}
			else
			{
				object->updateShaderCode(assetManager, rhi::ShaderStage::Fragment, "shaders/deferred.frag.spv");
			}
			
			object->registerDescriptor(rhi::DescriptorType::Uniform_Buffer, rhi::ShaderStage::Vertex | rhi::ShaderStage::Fragment, sceneUniformBuffer);
			object->registerDescriptor(rhi::DescriptorType::Combined_Image_Sampler, rhi::ShaderStage::Fragment, gBufferA);
			object->registerDescriptor(rhi::DescriptorType::Combined_Image_Sampler, rhi::ShaderStage::Fragment, gBufferB);
			object->registerDescriptor(rhi::DescriptorType::Combined_Image_Sampler, rhi::ShaderStage::Fragment, gBufferC);
			object->registerDescriptor(rhi::DescriptorType::Combined_Image_Sampler, rhi::ShaderStage::Fragment, sceneDepth);
			object->registerDescriptor(rhi::DescriptorType::Combined_Image_Sampler, rhi::ShaderStage::Fragment, shadowMapTexture);
			object->instantiate(context, glm::mat4(1.f));
		}
	}

	initSurfaceRenderpass(context, assetManager, sceneColor);
}
}