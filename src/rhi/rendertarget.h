#pragma once

#include <array>
#include <vector>
#include "platform/utils.h"
#include "rhi/resources.h"
#include "vulkan/context.h"
#include "rhi/texture.h"
#include "rhi/transition.h"

namespace rhi
{
class Context;

const uint8_t INVALID_ATTACHMENT_INDEX = -1;

typedef uint64_t AttachmentHash;

class Attachment
{
public:
    Attachment(Texture* texture, AttachmentOp loadOp, AttachmentOp storeOp, glm::vec4 clearValue);
    Attachment(Texture* texture, AttachmentOp loadOp, AttachmentOp storeOp, AttachmentOp subLoadOp, AttachmentOp subStoreOp, glm::vec4 clearValue);

    AttachmentOp getLoadOp() { return loadOp; }
    AttachmentOp getStoreOp() { return storeOp; }
    AttachmentOp getSubLoadOp() { return subLoadOp; }
    AttachmentOp getSubStoreOp() { return subStoreOp; }

    Texture* getTexture() { return texture; }
    glm::vec4 getClearValue() { return clearValue; }
private:
    Texture* texture;
    AttachmentOp loadOp;
    AttachmentOp storeOp;
    AttachmentOp subLoadOp;
    AttachmentOp subStoreOp;
    glm::vec4 clearValue;
};

class Subpass
{
public:
    Subpass()
        :depthAttachment(INVALID_ATTACHMENT_INDEX)
    {

    }

    void addColorAttachment(uint8_t index)
    {
        colorAttachments.push_back(index);
    }

    void setDepthStencilAttachment(uint8_t index)
    {
        ASSERT(index == INVALID_ATTACHMENT_INDEX);
        depthAttachment = index;
    }

    std::vector<uint8_t>& getColorAttachments() { return colorAttachments; }
    std::vector<uint8_t>& getInputAttachments() { return inputAttachments; }

    uint8_t getDepthAttachment() { return depthAttachment; }
protected:
    // resolve
    std::vector<uint8_t> colorAttachments;
    std::vector<uint8_t> inputAttachments; // Don't use yet
    uint8_t depthAttachment;
};

class RenderTarget
{
public:
    RenderTarget(uint16_t _width, uint16_t _height)
        : width(_width)
        , height(_height)
        , attachmentCount(0)
        , depthStencilAttachment(nullptr)
    {

    }

    virtual ~RenderTarget() = default;

    virtual void destroy(Context* context)
    {
        for (auto& attachment : attachments)
        {
            delete attachment;
        }

        for (auto& subpass : subpasses)
        {
            delete subpass;
        }
    }

    virtual void build(Context* context) = 0;

    virtual void addTransition(Context* context, Transition* transition) = 0;

    virtual void flushTransition(rhi::Context* context) = 0;

    uint32_t addColorAttachment(Attachment* attachment)
    {
        attachments.push_back(attachment);
        return attachmentCount++;
    }

    uint32_t addDepthAttachment(Attachment* attachment)
    {
        depthStencilAttachment = attachment;
        return attachmentCount++;
    }

    uint32_t addSubpass(Subpass* subpass)
    {
        subpasses.push_back(subpass);
        return static_cast<uint32_t>(subpasses.size() - 1);
    }
public:
    virtual bool begin(Context* context) = 0;
    virtual bool end(Context* context) = 0;
protected:
    std::vector<Attachment*>& getAttachments() { return attachments; }
    std::vector<Subpass*>& getSubpasses() { return subpasses; }
protected:
    const uint16_t width;
    const uint16_t height;
    std::vector<Attachment*> attachments;
    Attachment* depthStencilAttachment;
    std::vector<Subpass*> subpasses;
    uint32_t attachmentCount;
};
}