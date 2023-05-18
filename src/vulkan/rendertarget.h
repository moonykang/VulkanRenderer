#pragma once

#include <vector>

#include "rhi/rendertarget.h"
#include "vulkan/resources.h"
#include "vulkan/vk_wrapper.h"
#include "vulkan/framebuffer.h"

namespace rhi
{
class Context;
}

namespace vk
{
class Framebuffer;

class SubpassDescription
{
public:
    SubpassDescription();

    ~SubpassDescription();

    void  init(rhi::Subpass* subpass, VkSubpassDescription* subpassDescription);
private:
    
    std::vector<VkAttachmentReference> colorAttachmentReferences;
    std::vector<VkAttachmentReference> inputAttachmentReferences;
    VkAttachmentReference* depthAttachmentReference;
};

class Renderpass
{
public:
    void init(VkDevice device
        , std::vector<VkAttachmentDescription>* attachmentDescriptions
        , std::vector<VkSubpassDescription>* subpassDescriptions
        , std::vector<VkSubpassDependency>* subpassDependencies);

    void destroy(VkDevice device);

public:
    VkRenderPass getRenderpass();
private:
    handle::RenderPass renderpass;
};

class RenderTarget : public rhi::RenderTarget
{
public:
    RenderTarget(uint16_t width, uint16_t height);

    virtual void destroy(rhi::Context* context) override;

    void buildRenderpass(rhi::Context* context, std::vector<VkAttachmentDescription>& attachmentDescriptions, std::vector<rhi::Subpass*>& subpasses);

    virtual void build(rhi::Context* context) override;

public:
    void prepareBegin(CommandBuffer* commandBuffer);

    virtual bool begin(rhi::Context* context) override;

    virtual bool end(rhi::Context* context) override;

    void addTransition(rhi::Context* context, rhi::Transition* transition) override;

    void flushTransition(rhi::Context* context) override;
public:
    VkRenderPass getRenderpass();
protected:
    void updateAttachmentDescriptions(Context* context,
                                      std::vector<VkAttachmentDescription>* attachmentDescriptions,
                                      std::vector<VkImageView>& attachmentViews);
protected:
    Renderpass renderpass;
    Framebuffer framebuffer;
    std::vector<vk::Image*> images;
    VkRect2D renderArea;
    std::vector<VkClearValue> clearValues;
};

class SurfaceRenderTarget : public RenderTarget
{
public:
    SurfaceRenderTarget(uint16_t width, uint16_t height);

    virtual void destroy(rhi::Context* context) override;

    void build(rhi::Context* context) override;

public:
    void prepareBegin(CommandBuffer* commandBuffer, uint32_t imageindex);

    bool begin(rhi::Context* context) override final;
private:
    std::vector<Framebuffer> framebuffers;
    std::vector<Image*> swapchainImages;
};

class NullRenderTarget : public RenderTarget
{
public:
    NullRenderTarget(uint16_t width, uint16_t height);

    void destroy(rhi::Context* context) override;

    void build(rhi::Context* context) override;

    bool begin(rhi::Context* context) override;

    bool end(rhi::Context* context) override;
};
}