#include "rhi/pipeline.h"
#include "rhi/descriptor.h"

namespace rhi
{
PipelineState::PipelineState(FrontFace frontFace
							, CullMode cullMode
							, PolygonMode polygonMode
							, uint32_t tessellationPatchControl
							, std::vector<ColorBlendMaskFlags> colorBlendMaskFlags
							, Topology topology
							, DepthStencilState depthStencilState)
	: frontFace(frontFace)
	, cullMode(cullMode)
	, polygonMode(polygonMode)
	, tessellationPatchControl(tessellationPatchControl)
	, topology(topology)
	, depthStencilState(depthStencilState)
{
	for (auto& colorBlendMaskFlag : colorBlendMaskFlags)
	{
		colorBlendMasks.push_back(colorBlendMaskFlag);
	}
}

FrontFace PipelineState::getFrontFace()
{
	return frontFace;
}

CullMode PipelineState::getCullMode()
{
	return cullMode;
}

PolygonMode PipelineState::getPolygonMode()
{
	return polygonMode;
}

uint32_t PipelineState::getTessellationPatchControl()
{
	return tessellationPatchControl;
}

std::vector<ColorBlendMaskFlags>& PipelineState::getColorBlendMasks()
{
	return colorBlendMasks;
}

Topology PipelineState::getTopology()
{
	return topology;
}

PipelineState::DepthStencilState& PipelineState::getDepthStencilState()
{
	return depthStencilState;
}

void ShaderModuleContainer::updateShaderCode(platform::AssetManager* assetManager, rhi::ShaderStage shaderStage, std::string path)
{
	util::MemoryBuffer shaderCode;
	assetManager->readFile(path, &shaderCode);

	if (shaderCode.empty())
	{
		return;
	}

	auto& shader = shaders.emplace_back();
	shader.shaderStage = shaderStage;
	shader.code = std::move(shaderCode);
}

void ShaderModuleContainer::init(platform::AssetManager* assetManager, std::string path)
{
	updateShaderCode(assetManager, rhi::ShaderStage::Vertex, path + "/vert.spv");
	updateShaderCode(assetManager, rhi::ShaderStage::TessellationControl, path + "/tesc.spv");
	updateShaderCode(assetManager, rhi::ShaderStage::TessellationEvaluation, path + "/tese.spv");
	updateShaderCode(assetManager, rhi::ShaderStage::Fragment, path + "/frag.spv");
	updateShaderCode(assetManager, rhi::ShaderStage::RayGen, path + "/rgen.spv");
	updateShaderCode(assetManager, rhi::ShaderStage::RayMiss, path + "/rmiss.spv");
	updateShaderCode(assetManager, rhi::ShaderStage::ClosestHit, path + "/rchit.spv");
}
}