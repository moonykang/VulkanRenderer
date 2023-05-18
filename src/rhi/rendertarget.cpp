#include "rhi/rendertarget.h"

namespace rhi
{
Attachment::Attachment(Texture* texture, AttachmentOp loadOp, AttachmentOp storeOp, glm::vec4 clearValue)
    : Attachment(texture, loadOp, storeOp, AttachmentOp::Pass, AttachmentOp::Pass, clearValue)
{

}
Attachment::Attachment(Texture* texture, AttachmentOp loadOp, AttachmentOp storeOp, AttachmentOp subLoadOp, AttachmentOp subStoreOp, glm::vec4 clearValue)
    : texture(texture)
    , loadOp(loadOp)
    , storeOp(storeOp)
    , subLoadOp(subLoadOp)
    , subStoreOp(subStoreOp)
    , clearValue(clearValue)
{

}
}