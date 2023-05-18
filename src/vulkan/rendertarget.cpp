#include "rhi/context.h"
#include "vulkan/rendertarget.h"
#include "vulkan/commandBuffer.h"
#include "vulkan/surface.h"
#include "vulkan/texture.h"
#include "vulkan/buffer.h"

namespace vk
{
SubpassDescription::SubpassDescription()
    : depthAttachmentReference(nullptr)
{

}

SubpassDescription::~SubpassDescription()
{
    if (depthAttachmentReference != nullptr)
    {
        delete depthAttachmentReference;
        depthAttachmentReference = nullptr;
    }
}

void SubpassDescription::init(rhi::Subpass* subpass, VkSubpassDescription* subpassDescription)
{
    uint8_t depthAttachment = subpass->getDepthAttachment();

    for (auto& attachment : subpass->getColorAttachments())
    {
        colorAttachmentReferences.push_back({ attachment, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL });
    }

    for (auto& attachment : subpass->getInputAttachments())
    {
        if (attachment != depthAttachment)
        {
            inputAttachmentReferences.push_back({
                    attachment, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL });
        }
        else
        {
            inputAttachmentReferences.push_back({
                    attachment, VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL });
        }
    }

    if (depthAttachment != rhi::INVALID_ATTACHMENT_INDEX)
    {
        depthAttachmentReference = new VkAttachmentReference();
        depthAttachmentReference->attachment = depthAttachment;
        depthAttachmentReference->layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    }

    *subpassDescription = {};
    subpassDescription->pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpassDescription->colorAttachmentCount = static_cast<uint32_t>(colorAttachmentReferences.size());
    subpassDescription->pColorAttachments = colorAttachmentReferences.data();
    subpassDescription->pDepthStencilAttachment = depthAttachmentReference;
    subpassDescription->inputAttachmentCount = static_cast<uint32_t>(inputAttachmentReferences.size());;
    subpassDescription->pInputAttachments = inputAttachmentReferences.data();
    subpassDescription->preserveAttachmentCount = 0;
    subpassDescription->pPreserveAttachments = nullptr;
    subpassDescription->pResolveAttachments = nullptr;
}

void Renderpass::init(VkDevice device
    , std::vector<VkAttachmentDescription>* attachmentDescriptions
    , std::vector<VkSubpassDescription>* subpassDescriptions
    , std::vector<VkSubpassDependency>* subpassDependencies)
{
    VkRenderPassCreateInfo renderPassCreateInfo = {};
    renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassCreateInfo.attachmentCount = static_cast<uint32_t>(attachmentDescriptions->size());
    renderPassCreateInfo.pAttachments = attachmentDescriptions->data();
    renderPassCreateInfo.subpassCount = static_cast<uint32_t>(subpassDescriptions->size());
    renderPassCreateInfo.pSubpasses = subpassDescriptions->data();
    renderPassCreateInfo.dependencyCount = static_cast<uint32_t>(subpassDependencies->size());
    renderPassCreateInfo.pDependencies = subpassDependencies->data();

    VKCALL(renderpass.init(device, renderPassCreateInfo));
}

void Renderpass::destroy(VkDevice device)
{
    renderpass.destroy(device);
}

VkRenderPass Renderpass::getRenderpass()
{
    return renderpass.getHandle();
}

RenderTarget::RenderTarget(uint16_t width, uint16_t height)
    : rhi::RenderTarget(width, height)
{

}

void RenderTarget::destroy(rhi::Context* context)
{
    Context* contextVk = reinterpret_cast<Context*>(context);
    rhi::RenderTarget::destroy(context);

    for (auto& image : images)
    {
        image->destroy(contextVk->getDevice());
    }
    framebuffer.destroy(contextVk->getDevice());
    renderpass.destroy(contextVk->getDevice());
}

void RenderTarget::buildRenderpass(rhi::Context* context, std::vector<VkAttachmentDescription>& attachmentDescriptions, std::vector<rhi::Subpass*>& subpasses)
{
    Context* contextVk = reinterpret_cast<Context*>(context);

    std::vector<VkSubpassDescription> subpassDescriptions;
    std::vector<VkSubpassDependency> subpassDependencies;
    std::vector<SubpassDescription> subpassDescriptionDatas;

    for (auto& subpass : subpasses)
    {
        auto& subpassDescription = subpassDescriptions.emplace_back();
        auto& subpassDescriptionData = subpassDescriptionDatas.emplace_back();
        subpassDescriptionData.init(subpass, &subpassDescription);
    }

    renderpass.init(contextVk->getDevice(), &attachmentDescriptions, &subpassDescriptions, &subpassDependencies);
}

void RenderTarget::build(rhi::Context* context)
{
    Context* contextVk = reinterpret_cast<Context*>(context);
    renderArea = { 0, 0, width, height };
    
    std::vector<VkAttachmentDescription> attachmentDescriptions;
    std::vector<VkImageView> framebufferViews;

    updateAttachmentDescriptions(contextVk, &attachmentDescriptions, framebufferViews);

    buildRenderpass(context, attachmentDescriptions, subpasses);
    framebuffer.init(contextVk->getDevice(), renderpass.getRenderpass(), framebufferViews, renderArea.extent);
}

void RenderTarget::prepareBegin(CommandBuffer* commandBuffer)
{
    for (auto& image : images)
    {
        if ((image->getImageAspect() & VK_IMAGE_ASPECT_COLOR_BIT) != 0)
        {
            commandBuffer->addTransition(image->updateImageLayoutAndBarrier(ImageLayout::ColorAttachment));
        }
        else
        {
            commandBuffer->addTransition(image->updateImageLayoutAndBarrier(ImageLayout::DepthStencilAttachment));
        }
    }
}

ImageLayout getAttachmentImageLayout(VkFormat format, rhi::MemoryAccessFlags accessFlags)
{
    VkImageAspectFlags aspectFlags = getImageAspectMask(format);

    // Color
    if ((aspectFlags & VK_IMAGE_ASPECT_COLOR_BIT) != 0)
    {
        if (accessFlags & rhi::MemoryAccess::Read)
        {
            return ImageLayout::FragmentShaderReadOnly;
        }
        if (accessFlags & rhi::MemoryAccess::Write)
        {
            return ImageLayout::ColorAttachment;
        }
        if (accessFlags & rhi::MemoryAccess::General)
        {
            return ImageLayout::ComputeShaderWrite;
        }
        
        UNREACHABLE();
    }
    // Depth-Stencil
    else
    {
        if (accessFlags & rhi::MemoryAccess::Read)
        {
            return ImageLayout::DepthStencilAttachmentReadOnly;
        }
        if (accessFlags & rhi::MemoryAccess::Write)
        {
            return ImageLayout::DepthStencilAttachment;
        }
        if (accessFlags & rhi::MemoryAccess::General)
        {
            return ImageLayout::ComputeShaderWrite;
        }
        
        UNREACHABLE();
    }
}

VkImageLayout getAttachmentVkImageLayout(VkFormat format)
{
    VkImageAspectFlags aspectFlags = getImageAspectMask(format);
    if ((aspectFlags | VK_IMAGE_ASPECT_COLOR_BIT) != 0)
    {
        return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    }
    else
    {
        // TODO: Consider separate layout
        return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    }
}

void RenderTarget::addTransition(rhi::Context* context, rhi::Transition* transition)
{
    Context* contextVk = reinterpret_cast<Context*>(context);
    rhi::Texture* rhiTexture = transition->getTexture();
    rhi::StorageBuffer* rhiStorageBuffer = transition->getBuffer();

    if (rhiTexture)
    {        
        Texture* texture = reinterpret_cast<Texture*>(rhiTexture);
        ImageLayout imageLayout = getAttachmentImageLayout(texture->getFormat(), transition->getMemoryAccessFlags());
        contextVk->getActiveCommandBuffer()->addTransition(texture->updateImageLayoutAndBarrier(imageLayout));
    }

    if (rhiStorageBuffer)
    {
        StorageBuffer* buffer = reinterpret_cast<StorageBuffer*>(rhiStorageBuffer);
    }
}

void RenderTarget::flushTransition(rhi::Context* context)
{
    Context* contextVk = reinterpret_cast<Context*>(context);
    contextVk->getActiveCommandBuffer()->flushTransitions();
}

bool RenderTarget::begin(rhi::Context* context)
{
    Context* contextVk = reinterpret_cast<Context*>(context);
    CommandBuffer* commandBuffer = contextVk->getActiveCommandBuffer();
    prepareBegin(commandBuffer);

    VkRenderPassBeginInfo renderpassBeginInfo = {};
    renderpassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderpassBeginInfo.pNext = nullptr;
    renderpassBeginInfo.renderPass = renderpass.getRenderpass();
    renderpassBeginInfo.framebuffer = framebuffer.getFramebuffer();
    renderpassBeginInfo.renderArea = renderArea;

    // TODO
    renderpassBeginInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderpassBeginInfo.pClearValues = clearValues.data();

    commandBuffer->beginRenderPass(renderpassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

    VkViewport viewport = {};
    viewport.x = static_cast<float>(renderArea.offset.x);
    viewport.y = static_cast<float>(renderArea.offset.y);
    viewport.width = static_cast<float>(renderArea.extent.width);
    viewport.height = static_cast<float>(renderArea.extent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    commandBuffer->setViewport(viewport);
    VkRect2D scissor = renderArea;
    commandBuffer->setScissor(scissor);

    return true;
}

bool RenderTarget::end(rhi::Context* context)
{
    Context* contextVk = reinterpret_cast<Context*>(context);
    CommandBuffer* commandBuffer = contextVk->getActiveCommandBuffer();
    commandBuffer->endRenderPass();
    return true;
}

VkRenderPass RenderTarget::getRenderpass() { return renderpass.getRenderpass(); }


void RenderTarget::updateAttachmentDescriptions(Context* context,
                                                std::vector<VkAttachmentDescription>* attachmentDescriptions,
                                                std::vector<VkImageView>& attachmentViews)
{
    for (auto& attachment : attachments)
    {
        VkAttachmentDescription attachmentDescription = {};
        attachmentDescription.loadOp = convertToLoadOp(attachment->getLoadOp());
        attachmentDescription.storeOp = convertToStoreOp(attachment->getStoreOp());
        attachmentDescription.stencilLoadOp = convertToLoadOp(attachment->getSubLoadOp());
        attachmentDescription.stencilStoreOp = convertToStoreOp(attachment->getSubStoreOp());

        if (attachment->getTexture() == nullptr)
        {
            attachmentDescription.format = context->getSurface()->getFormat();
            attachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
            attachmentDescription.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            attachmentDescription.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        }
        else
        {
            Texture* texture = reinterpret_cast<Texture*>(attachment->getTexture());
            VkFormat format = texture->getFormat();
            VkImageLayout attachmentImageLayout = getAttachmentVkImageLayout(format);
            attachmentDescription.format = format;
            attachmentDescription.samples = static_cast<VkSampleCountFlagBits>(texture->getSamples());
            attachmentDescription.initialLayout = attachmentImageLayout;
            attachmentDescription.finalLayout = attachmentImageLayout;
            attachmentViews.push_back(texture->getImageView());
        }
        attachmentDescriptions->push_back(attachmentDescription);

        glm::vec4 attachmentClearValue = attachment->getClearValue();
        auto& clearValue = clearValues.emplace_back();
        clearValue.color = { attachmentClearValue.x, attachmentClearValue.y, attachmentClearValue.z, attachmentClearValue.a };
    }

    if (depthStencilAttachment)
    {
        VkAttachmentDescription attachmentDescription = {};
        attachmentDescription.loadOp = convertToLoadOp(depthStencilAttachment->getLoadOp());
        attachmentDescription.storeOp = convertToStoreOp(depthStencilAttachment->getStoreOp());
        attachmentDescription.stencilLoadOp = convertToLoadOp(depthStencilAttachment->getSubLoadOp());
        attachmentDescription.stencilStoreOp = convertToStoreOp(depthStencilAttachment->getSubStoreOp());

        if (depthStencilAttachment->getTexture() == nullptr)
        {
            attachmentDescription.format = context->getSurface()->getFormat();
            attachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
            attachmentDescription.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            attachmentDescription.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        }
        else
        {
            Texture* texture = reinterpret_cast<Texture*>(depthStencilAttachment->getTexture());
            VkFormat format = texture->getFormat();
            VkImageLayout attachmentImageLayout = getAttachmentVkImageLayout(format);
            attachmentDescription.format = format;
            attachmentDescription.samples = static_cast<VkSampleCountFlagBits>(texture->getSamples());
            attachmentDescription.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            attachmentDescription.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            attachmentViews.push_back(texture->getImageView());
        }
        attachmentDescriptions->push_back(attachmentDescription);

        glm::vec4 attachmentClearValue = depthStencilAttachment->getClearValue();
        auto& clearValue = clearValues.emplace_back();
        //TODO
        clearValue.depthStencil = { attachmentClearValue.x, 0 };
    }
}

SurfaceRenderTarget::SurfaceRenderTarget(uint16_t width, uint16_t height)
    : RenderTarget(width, height)
{

}

void SurfaceRenderTarget::destroy(rhi::Context* context)
{
    RenderTarget::destroy(context);
    Context* contextVk = reinterpret_cast<Context*>(context);

    for (auto& framebuffer : framebuffers)
    {
        framebuffer.destroy(contextVk->getDevice());
    }
}

void SurfaceRenderTarget::build(rhi::Context* context)
{
    Context* contextVk = reinterpret_cast<Context*>(context);
    renderArea = { 0, 0, width, height };
    std::vector<VkImageView> attachmentViews;
    std::vector<VkAttachmentDescription> attachmentDescriptions;
    updateAttachmentDescriptions(contextVk, &attachmentDescriptions, attachmentViews);
    buildRenderpass(context, attachmentDescriptions, subpasses);

    swapchainImages = contextVk->getSurface()->getSwapchainImages();
    for (size_t i = 0; i < swapchainImages.size(); i++)
    {
        std::vector<VkImageView> framebufferViews;
        framebufferViews.push_back(swapchainImages[i]->getImageView());
        for (auto& imageView : attachmentViews)
        {
            framebufferViews.push_back(imageView);
        }
        auto& framebuffer = framebuffers.emplace_back();
        framebuffer.init(contextVk->getDevice(), renderpass.getRenderpass(), framebufferViews, { width, height });
    }
}

void SurfaceRenderTarget::prepareBegin(CommandBuffer* commandBuffer, uint32_t imageindex)
{
    RenderTarget::prepareBegin(commandBuffer);
    commandBuffer->addTransition(swapchainImages[imageindex]->updateImageLayoutAndBarrier(ImageLayout::ColorAttachment));
}

bool SurfaceRenderTarget::begin(rhi::Context* context)
{
    Context* contextVk = reinterpret_cast<Context*>(context);
    CommandBuffer* commandBuffer = contextVk->getActiveCommandBuffer();
    uint32_t imageIndex = contextVk->getNextImageIndex();
    prepareBegin(commandBuffer, imageIndex);

    VkRenderPassBeginInfo renderpassBeginInfo = {};
    renderpassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderpassBeginInfo.pNext = nullptr;
    renderpassBeginInfo.renderPass = renderpass.getRenderpass();
    renderpassBeginInfo.framebuffer = framebuffers[imageIndex].getFramebuffer();
    renderpassBeginInfo.renderArea = renderArea;

    // TODO
    renderpassBeginInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderpassBeginInfo.pClearValues = clearValues.data();

    commandBuffer->beginRenderPass(renderpassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

    VkViewport viewport = {};
    viewport.x = static_cast<float>(renderArea.offset.x);
    viewport.y = static_cast<float>(renderArea.offset.y);
    viewport.width = static_cast<float>(renderArea.extent.width);
    viewport.height = static_cast<float>(renderArea.extent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    commandBuffer->setViewport(viewport);
    VkRect2D scissor = renderArea;
    commandBuffer->setScissor(scissor);
    return true;
}

NullRenderTarget::NullRenderTarget(uint16_t width, uint16_t height)
    : RenderTarget(width, height)
{

}

void NullRenderTarget::destroy(rhi::Context* context)
{
}

void NullRenderTarget::build(rhi::Context* context)
{

}

bool NullRenderTarget::begin(rhi::Context* context)
{
    Context* contextVk = reinterpret_cast<Context*>(context);
    CommandBuffer* commandBuffer = contextVk->getActiveCommandBuffer();
    commandBuffer->flushTransitions();

    return true;
}

bool NullRenderTarget::end(rhi::Context* context)
{
    return true;
}
}