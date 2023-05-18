#pragma once

#include <map>
#include "rhi/resources.h"
#include "vk_headers.h"

namespace vk
{

using rhi::ImageLayout;
using rhi::Format;

inline VkColorComponentFlags convertToColorComponentFlags(rhi::ColorBlendMaskFlags colorBlendMaskFlags)
{
    VkColorComponentFlags colorComponentFlags = 0;

    if ((colorBlendMaskFlags & rhi::ColorBlendMask::COLOR_COMPONENT_R_BIT) != 0)
    {
        colorComponentFlags |= VK_COLOR_COMPONENT_R_BIT;
    }

    if ((colorBlendMaskFlags & rhi::ColorBlendMask::COLOR_COMPONENT_G_BIT) != 0)
    {
        colorComponentFlags |= VK_COLOR_COMPONENT_G_BIT;
    }

    if ((colorBlendMaskFlags & rhi::ColorBlendMask::COLOR_COMPONENT_B_BIT) != 0)
    {
        colorComponentFlags |= VK_COLOR_COMPONENT_B_BIT;
    }

    if ((colorBlendMaskFlags & rhi::ColorBlendMask::COLOR_COMPONENT_A_BIT) != 0)
    {
        colorComponentFlags |= VK_COLOR_COMPONENT_A_BIT;
    }

    return colorComponentFlags;
}

inline VkAttachmentStoreOp convertToStoreOp(rhi::AttachmentOp attachmentOp)
{
    switch (attachmentOp)
    {
        case rhi::AttachmentOp::Pass:
            return VK_ATTACHMENT_STORE_OP_DONT_CARE;
        case rhi::AttachmentOp::Store:
            return VK_ATTACHMENT_STORE_OP_STORE;
        default:
            UNREACHABLE();
            return VK_ATTACHMENT_STORE_OP_DONT_CARE;
    }
}

inline VkAttachmentLoadOp convertToLoadOp(rhi::AttachmentOp attachmentOp)
{
    switch (attachmentOp)
    {
        case rhi::AttachmentOp::Pass:
            return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        case rhi::AttachmentOp::Clear:
            return VK_ATTACHMENT_LOAD_OP_CLEAR;
        case rhi::AttachmentOp::Load:
            return VK_ATTACHMENT_LOAD_OP_LOAD;
        default:
            UNREACHABLE();
            return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    }
}

// TODO
inline VkAccessFlags convertToVkAccessFlags(rhi::MemoryAccessFlags rhiAccessFlags)
{
    VkAccessFlags accessFlags = 0;

    if (rhiAccessFlags & rhi::MemoryAccess::Read)
    {
        accessFlags |= VK_ACCESS_SHADER_READ_BIT;
    }
    if (rhiAccessFlags & rhi::MemoryAccess::Write)
    {
        accessFlags |= VK_ACCESS_SHADER_WRITE_BIT;
    }
    if (rhiAccessFlags & rhi::MemoryAccess::Indirect)
    {
        accessFlags |= VK_ACCESS_INDIRECT_COMMAND_READ_BIT;
    }
    return accessFlags;
}
enum class ShaderStage : uint8_t
{
    Vertex = 0,
    Geometry = 1,
    Fragment = 2,
    Compute = 3
};

inline VkStencilOp convertToVkStencilOp(rhi::StencilOp stencilOp)
{
    switch (stencilOp)
    {
    case rhi::StencilOp::KEEP:
        return VK_STENCIL_OP_KEEP;
    case rhi::StencilOp::ZERO:
        return VK_STENCIL_OP_ZERO;
    case rhi::StencilOp::REPLACE:
        return VK_STENCIL_OP_REPLACE;
    case rhi::StencilOp::INCREMENT_AND_CLAMP:
        return VK_STENCIL_OP_INCREMENT_AND_CLAMP;
    case rhi::StencilOp::DECREMENT_AND_CLAMP:
        return VK_STENCIL_OP_DECREMENT_AND_CLAMP;
    case rhi::StencilOp::INVERT:
        return VK_STENCIL_OP_INVERT;
    case rhi::StencilOp::INCREMENT_AND_WRAP:
        return VK_STENCIL_OP_INCREMENT_AND_WRAP;
    case rhi::StencilOp::DECREMENT_AND_WRAP:
        return VK_STENCIL_OP_DECREMENT_AND_WRAP;
    }
}

inline VkCompareOp convertToVkCompareOp(rhi::CompareOp sompareOp)
{
    switch (sompareOp)
    {
    case rhi::CompareOp::NEVER:
        return VK_COMPARE_OP_NEVER;
    case rhi::CompareOp::LESS:
        return VK_COMPARE_OP_LESS;
    case rhi::CompareOp::EQUAL:
        return VK_COMPARE_OP_EQUAL;
    case rhi::CompareOp::LESS_OR_EQUAL:
        return VK_COMPARE_OP_LESS_OR_EQUAL;
    case rhi::CompareOp::GREATER:
        return VK_COMPARE_OP_GREATER;
    case rhi::CompareOp::NOT_EQUAL:
        return VK_COMPARE_OP_NOT_EQUAL;
    case rhi::CompareOp::GREATER_OR_EQUAL:
        return VK_COMPARE_OP_GREATER_OR_EQUAL;
    case rhi::CompareOp::ALWAYS:
        return VK_COMPARE_OP_ALWAYS;
    }
}


inline VkFilter convertToVkFilter(rhi::SampleMode sampleMode)
{
    switch (sampleMode)
    {
    case rhi::SampleMode::Nearest:
        return VK_FILTER_NEAREST;
    case rhi::SampleMode::Linear:
        return VK_FILTER_LINEAR;
    default:
        UNREACHABLE();
        return VK_FILTER_NEAREST;
    }
}

inline VkSamplerMipmapMode convertToVkSamplerMipmapMode(rhi::SampleMode sampleMode)
{
    switch (sampleMode)
    {
    case rhi::SampleMode::Nearest:
        return VK_SAMPLER_MIPMAP_MODE_NEAREST;
    case rhi::SampleMode::Linear:
        return VK_SAMPLER_MIPMAP_MODE_LINEAR;
    default:
        UNREACHABLE();
        return VK_SAMPLER_MIPMAP_MODE_NEAREST;
    }
}

inline VkShaderStageFlagBits convertToVkShaderStage(rhi::ShaderStage shaderStage)
{
    switch (shaderStage)
    {
    case rhi::ShaderStage::Vertex:
        return VK_SHADER_STAGE_VERTEX_BIT;
    case rhi::ShaderStage::Geometry:
        return VK_SHADER_STAGE_GEOMETRY_BIT;
    case rhi::ShaderStage::TessellationControl:
        return VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
    case rhi::ShaderStage::TessellationEvaluation:
        return VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
    case rhi::ShaderStage::Fragment:
        return VK_SHADER_STAGE_FRAGMENT_BIT;
    case rhi::ShaderStage::Compute:
        return VK_SHADER_STAGE_COMPUTE_BIT;
    case rhi::ShaderStage::RayGen:
        return VK_SHADER_STAGE_RAYGEN_BIT_KHR;
    case rhi::ShaderStage::RayMiss:
        return VK_SHADER_STAGE_MISS_BIT_KHR;
    case rhi::ShaderStage::ClosestHit:
        return VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
    default:
        UNREACHABLE();
        return VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM;
    }
}

inline VkShaderStageFlags convertToVkShaderStageFlag(rhi::ShaderStageFlags stageFlags)
{
    VkShaderStageFlags shaderStageFlags = 0;

    if ((stageFlags & rhi::ShaderStage::Vertex) != 0)
    {
        shaderStageFlags |= VK_SHADER_STAGE_VERTEX_BIT;
    }

    if ((stageFlags & rhi::ShaderStage::Geometry) != 0)
    {
        shaderStageFlags |= VK_SHADER_STAGE_GEOMETRY_BIT;
    }

    if ((stageFlags & rhi::ShaderStage::TessellationControl) != 0)
    {
        shaderStageFlags |= VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
    }

    if ((stageFlags & rhi::ShaderStage::TessellationEvaluation) != 0)
    {
        shaderStageFlags |= VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
    }

    if ((stageFlags & rhi::ShaderStage::Fragment) != 0)
    {
        shaderStageFlags |= VK_SHADER_STAGE_FRAGMENT_BIT;
    }

    if ((stageFlags & rhi::ShaderStage::Compute) != 0)
    {
        shaderStageFlags |= VK_SHADER_STAGE_COMPUTE_BIT;
    }

    if ((stageFlags & rhi::ShaderStage::RayGen) != 0)
    {
        shaderStageFlags |= VK_SHADER_STAGE_RAYGEN_BIT_KHR;
    }

    if ((stageFlags & rhi::ShaderStage::ClosestHit) != 0)
    {
        shaderStageFlags |= VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
    }

    return shaderStageFlags;
}

inline VkDescriptorType convertToVkDescriptorType(rhi::DescriptorType type)
{
    switch (type)
    {
    case rhi::DescriptorType::Sampler:
        return VK_DESCRIPTOR_TYPE_SAMPLER;
    case rhi::DescriptorType::Combined_Image_Sampler:
        return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    case rhi::DescriptorType::Sampled_Image:
        return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    case rhi::DescriptorType::Storage_Image:
        return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    case rhi::DescriptorType::Uniform_Texel_Buffer:
        return VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
    case rhi::DescriptorType::Storage_Texel_Buffer:
        return VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
    case rhi::DescriptorType::Uniform_Buffer:
        return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    case rhi::DescriptorType::Storage_Buffer:
        return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    case rhi::DescriptorType::Uniform_Buffer_Dynamic:
        return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    case rhi::DescriptorType::Storage_Buffer_Dynamic:
        return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
    case rhi::DescriptorType::Input_Attachment:
        return VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
    case rhi::DescriptorType::Acceleration_structure:
        return VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
    default:
        UNREACHABLE();
        return VK_DESCRIPTOR_TYPE_SAMPLER;
    }
}

struct FormatData
{
    VkFormat format;
    VkImageAspectFlags aspects;
};

static std::map<rhi::Format, FormatData> kFormatMap = {
    // Color
    {rhi::Format::A1R5G5B5_UNORM, { VK_FORMAT_A1R5G5B5_UNORM_PACK16, VK_IMAGE_ASPECT_COLOR_BIT }},        
    {rhi::Format::ASTC_10x10_SRGB_BLOCK, { VK_FORMAT_ASTC_10x10_SRGB_BLOCK, VK_IMAGE_ASPECT_COLOR_BIT }},
    {rhi::Format::ASTC_10x10_UNORM_BLOCK, { VK_FORMAT_ASTC_10x10_UNORM_BLOCK, VK_IMAGE_ASPECT_COLOR_BIT }},
    {rhi::Format::ASTC_10x5_SRGB_BLOCK, { VK_FORMAT_ASTC_10x5_SRGB_BLOCK, VK_IMAGE_ASPECT_COLOR_BIT }},
    {rhi::Format::ASTC_10x5_UNORM_BLOCK, { VK_FORMAT_ASTC_10x5_UNORM_BLOCK, VK_IMAGE_ASPECT_COLOR_BIT }},
    {rhi::Format::ASTC_10x6_SRGB_BLOCK, { VK_FORMAT_ASTC_10x6_SRGB_BLOCK, VK_IMAGE_ASPECT_COLOR_BIT }},
    {rhi::Format::ASTC_10x6_UNORM_BLOCK, { VK_FORMAT_ASTC_10x6_UNORM_BLOCK, VK_IMAGE_ASPECT_COLOR_BIT }},
    {rhi::Format::ASTC_10x8_SRGB_BLOCK, { VK_FORMAT_ASTC_10x8_SRGB_BLOCK, VK_IMAGE_ASPECT_COLOR_BIT }},
    {rhi::Format::ASTC_10x8_UNORM_BLOCK, { VK_FORMAT_ASTC_10x8_UNORM_BLOCK, VK_IMAGE_ASPECT_COLOR_BIT }},
    {rhi::Format::ASTC_12x10_SRGB_BLOCK, { VK_FORMAT_ASTC_12x10_SRGB_BLOCK, VK_IMAGE_ASPECT_COLOR_BIT }},
    {rhi::Format::ASTC_12x10_UNORM_BLOCK, { VK_FORMAT_ASTC_12x10_UNORM_BLOCK, VK_IMAGE_ASPECT_COLOR_BIT }},
    {rhi::Format::ASTC_12x12_SRGB_BLOCK, { VK_FORMAT_ASTC_12x12_SRGB_BLOCK, VK_IMAGE_ASPECT_COLOR_BIT }},
    {rhi::Format::ASTC_12x12_UNORM_BLOCK, { VK_FORMAT_ASTC_12x12_UNORM_BLOCK, VK_IMAGE_ASPECT_COLOR_BIT }},
    {rhi::Format::ASTC_4x4_SRGB_BLOCK, { VK_FORMAT_ASTC_4x4_SRGB_BLOCK, VK_IMAGE_ASPECT_COLOR_BIT }},
    {rhi::Format::ASTC_4x4_UNORM_BLOCK, { VK_FORMAT_ASTC_4x4_UNORM_BLOCK, VK_IMAGE_ASPECT_COLOR_BIT }},
    {rhi::Format::ASTC_5x4_SRGB_BLOCK, { VK_FORMAT_ASTC_5x4_SRGB_BLOCK, VK_IMAGE_ASPECT_COLOR_BIT }},
    {rhi::Format::ASTC_5x4_UNORM_BLOCK, { VK_FORMAT_ASTC_5x4_UNORM_BLOCK, VK_IMAGE_ASPECT_COLOR_BIT }},
    {rhi::Format::ASTC_5x5_SRGB_BLOCK, { VK_FORMAT_ASTC_5x5_SRGB_BLOCK, VK_IMAGE_ASPECT_COLOR_BIT }},
    {rhi::Format::ASTC_5x5_UNORM_BLOCK, { VK_FORMAT_ASTC_5x5_UNORM_BLOCK, VK_IMAGE_ASPECT_COLOR_BIT }},
    {rhi::Format::ASTC_6x5_SRGB_BLOCK, { VK_FORMAT_ASTC_6x5_SRGB_BLOCK, VK_IMAGE_ASPECT_COLOR_BIT }},
    {rhi::Format::ASTC_6x5_UNORM_BLOCK, { VK_FORMAT_ASTC_6x5_UNORM_BLOCK, VK_IMAGE_ASPECT_COLOR_BIT }},
    {rhi::Format::ASTC_6x6_SRGB_BLOCK, { VK_FORMAT_ASTC_6x6_SRGB_BLOCK, VK_IMAGE_ASPECT_COLOR_BIT }},
    {rhi::Format::ASTC_6x6_UNORM_BLOCK, { VK_FORMAT_ASTC_6x6_UNORM_BLOCK, VK_IMAGE_ASPECT_COLOR_BIT }},
    {rhi::Format::ASTC_8x5_SRGB_BLOCK, { VK_FORMAT_ASTC_8x5_SRGB_BLOCK, VK_IMAGE_ASPECT_COLOR_BIT }},
    {rhi::Format::ASTC_8x5_UNORM_BLOCK, { VK_FORMAT_ASTC_8x5_UNORM_BLOCK, VK_IMAGE_ASPECT_COLOR_BIT }},
    {rhi::Format::ASTC_8x6_SRGB_BLOCK, { VK_FORMAT_ASTC_8x6_SRGB_BLOCK, VK_IMAGE_ASPECT_COLOR_BIT }},
    {rhi::Format::ASTC_8x6_UNORM_BLOCK, { VK_FORMAT_ASTC_8x6_UNORM_BLOCK, VK_IMAGE_ASPECT_COLOR_BIT }},
    {rhi::Format::ASTC_8x8_SRGB_BLOCK, { VK_FORMAT_ASTC_8x8_SRGB_BLOCK, VK_IMAGE_ASPECT_COLOR_BIT }},
    {rhi::Format::ASTC_8x8_UNORM_BLOCK, { VK_FORMAT_ASTC_8x8_UNORM_BLOCK, VK_IMAGE_ASPECT_COLOR_BIT }},
    {rhi::Format::B10G10R10A2_UNORM, { VK_FORMAT_A2R10G10B10_UNORM_PACK32, VK_IMAGE_ASPECT_COLOR_BIT }},
    {rhi::Format::B4G4R4A4_UNORM, { VK_FORMAT_B4G4R4A4_UNORM_PACK16, VK_IMAGE_ASPECT_COLOR_BIT }},
    {rhi::Format::B5G5R5A1_UNORM, { VK_FORMAT_B5G5R5A1_UNORM_PACK16, VK_IMAGE_ASPECT_COLOR_BIT }},
    {rhi::Format::B5G6R5_UNORM, { VK_FORMAT_B5G6R5_UNORM_PACK16, VK_IMAGE_ASPECT_COLOR_BIT }},
    {rhi::Format::B8G8R8A8_UNORM, { VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT }},
    {rhi::Format::B8G8R8A8_UNORM_SRGB, { VK_FORMAT_B8G8R8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT }},
    {rhi::Format::BC1_RGBA_UNORM_BLOCK, { VK_FORMAT_BC1_RGBA_UNORM_BLOCK, VK_IMAGE_ASPECT_COLOR_BIT }},
    {rhi::Format::BC1_RGBA_UNORM_SRGB_BLOCK, { VK_FORMAT_BC1_RGBA_SRGB_BLOCK, VK_IMAGE_ASPECT_COLOR_BIT }},
    {rhi::Format::BC1_RGB_UNORM_BLOCK, { VK_FORMAT_BC1_RGB_UNORM_BLOCK, VK_IMAGE_ASPECT_COLOR_BIT }},
    {rhi::Format::BC1_RGB_UNORM_SRGB_BLOCK, { VK_FORMAT_BC1_RGB_SRGB_BLOCK, VK_IMAGE_ASPECT_COLOR_BIT }},
    {rhi::Format::BC2_RGBA_UNORM_BLOCK, { VK_FORMAT_BC2_UNORM_BLOCK, VK_IMAGE_ASPECT_COLOR_BIT }},
    {rhi::Format::BC2_RGBA_UNORM_SRGB_BLOCK, { VK_FORMAT_BC2_SRGB_BLOCK, VK_IMAGE_ASPECT_COLOR_BIT }},
    {rhi::Format::BC3_RGBA_UNORM_BLOCK, { VK_FORMAT_BC3_UNORM_BLOCK, VK_IMAGE_ASPECT_COLOR_BIT }},
    {rhi::Format::BC3_RGBA_UNORM_SRGB_BLOCK, { VK_FORMAT_BC3_SRGB_BLOCK, VK_IMAGE_ASPECT_COLOR_BIT }},
    {rhi::Format::BC4_RED_SNORM_BLOCK, { VK_FORMAT_BC4_SNORM_BLOCK, VK_IMAGE_ASPECT_COLOR_BIT }},
    {rhi::Format::BC4_RED_UNORM_BLOCK, { VK_FORMAT_BC4_UNORM_BLOCK, VK_IMAGE_ASPECT_COLOR_BIT }},
    {rhi::Format::BC5_RG_SNORM_BLOCK, { VK_FORMAT_BC5_SNORM_BLOCK, VK_IMAGE_ASPECT_COLOR_BIT }},
    {rhi::Format::BC5_RG_UNORM_BLOCK, { VK_FORMAT_BC5_UNORM_BLOCK, VK_IMAGE_ASPECT_COLOR_BIT }},
    {rhi::Format::BC6H_RGB_SFLOAT_BLOCK, { VK_FORMAT_BC6H_SFLOAT_BLOCK, VK_IMAGE_ASPECT_COLOR_BIT }},
    {rhi::Format::BC6H_RGB_UFLOAT_BLOCK, { VK_FORMAT_BC6H_UFLOAT_BLOCK, VK_IMAGE_ASPECT_COLOR_BIT }},
    {rhi::Format::BC7_RGBA_UNORM_BLOCK, { VK_FORMAT_BC7_UNORM_BLOCK, VK_IMAGE_ASPECT_COLOR_BIT }},
    {rhi::Format::BC7_RGBA_UNORM_SRGB_BLOCK, { VK_FORMAT_BC7_SRGB_BLOCK, VK_IMAGE_ASPECT_COLOR_BIT }},
    {rhi::Format::EAC_R11G11_SNORM_BLOCK, { VK_FORMAT_EAC_R11G11_SNORM_BLOCK, VK_IMAGE_ASPECT_COLOR_BIT }},
    {rhi::Format::EAC_R11G11_UNORM_BLOCK, { VK_FORMAT_EAC_R11G11_UNORM_BLOCK, VK_IMAGE_ASPECT_COLOR_BIT }},
    {rhi::Format::EAC_R11_SNORM_BLOCK, { VK_FORMAT_EAC_R11_SNORM_BLOCK, VK_IMAGE_ASPECT_COLOR_BIT }},
    {rhi::Format::EAC_R11_UNORM_BLOCK, { VK_FORMAT_EAC_R11_UNORM_BLOCK, VK_IMAGE_ASPECT_COLOR_BIT }},
    {rhi::Format::ETC2_R8G8B8A1_SRGB_BLOCK, { VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK, VK_IMAGE_ASPECT_COLOR_BIT }},
    {rhi::Format::ETC2_R8G8B8A1_UNORM_BLOCK, { VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK, VK_IMAGE_ASPECT_COLOR_BIT }},
    {rhi::Format::ETC2_R8G8B8A8_SRGB_BLOCK, { VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK, VK_IMAGE_ASPECT_COLOR_BIT }},
    {rhi::Format::ETC2_R8G8B8A8_UNORM_BLOCK, { VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK, VK_IMAGE_ASPECT_COLOR_BIT }},
    {rhi::Format::ETC2_R8G8B8_SRGB_BLOCK, { VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK, VK_IMAGE_ASPECT_COLOR_BIT }},
    {rhi::Format::ETC2_R8G8B8_UNORM_BLOCK, { VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK, VK_IMAGE_ASPECT_COLOR_BIT }},
    {rhi::Format::G8_B8R8_2PLANE_420_UNORM, { VK_FORMAT_G8_B8R8_2PLANE_420_UNORM, VK_IMAGE_ASPECT_COLOR_BIT }},
    {rhi::Format::G8_B8_R8_3PLANE_420_UNORM, { VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM, VK_IMAGE_ASPECT_COLOR_BIT }},
    {rhi::Format::NONE, { VK_FORMAT_UNDEFINED, VK_IMAGE_ASPECT_COLOR_BIT }},
    {rhi::Format::R10G10B10A2_SINT, { VK_FORMAT_A2B10G10R10_SINT_PACK32, VK_IMAGE_ASPECT_COLOR_BIT }},
    {rhi::Format::R10G10B10A2_SNORM, { VK_FORMAT_A2B10G10R10_SNORM_PACK32, VK_IMAGE_ASPECT_COLOR_BIT }},
    {rhi::Format::R10G10B10A2_SSCALED, { VK_FORMAT_A2B10G10R10_SSCALED_PACK32, VK_IMAGE_ASPECT_COLOR_BIT }},
    {rhi::Format::R10G10B10A2_UINT, { VK_FORMAT_A2B10G10R10_UINT_PACK32, VK_IMAGE_ASPECT_COLOR_BIT }},
    {rhi::Format::R10G10B10A2_UNORM, { VK_FORMAT_A2B10G10R10_UNORM_PACK32, VK_IMAGE_ASPECT_COLOR_BIT }},
    {rhi::Format::R10G10B10A2_USCALED, { VK_FORMAT_A2B10G10R10_USCALED_PACK32, VK_IMAGE_ASPECT_COLOR_BIT }},
    {rhi::Format::R11G11B10_FLOAT, { VK_FORMAT_B10G11R11_UFLOAT_PACK32, VK_IMAGE_ASPECT_COLOR_BIT }},
    {rhi::Format::R16G16B16A16_FLOAT, { VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_ASPECT_COLOR_BIT }},
    {rhi::Format::R16G16B16A16_SINT, { VK_FORMAT_R16G16B16A16_SINT, VK_IMAGE_ASPECT_COLOR_BIT }},
    {rhi::Format::R16G16B16A16_SNORM, { VK_FORMAT_R16G16B16A16_SNORM, VK_IMAGE_ASPECT_COLOR_BIT }},
    {rhi::Format::R16G16B16A16_SSCALED, { VK_FORMAT_R16G16B16A16_SSCALED, VK_IMAGE_ASPECT_COLOR_BIT }},
    {rhi::Format::R16G16B16A16_UINT, { VK_FORMAT_R16G16B16A16_UINT, VK_IMAGE_ASPECT_COLOR_BIT }},
    {rhi::Format::R16G16B16A16_UNORM, { VK_FORMAT_R16G16B16A16_UNORM, VK_IMAGE_ASPECT_COLOR_BIT }},
    {rhi::Format::R16G16B16A16_USCALED, { VK_FORMAT_R16G16B16A16_USCALED, VK_IMAGE_ASPECT_COLOR_BIT }},
    {rhi::Format::R16G16B16_FLOAT, { VK_FORMAT_R16G16B16_SFLOAT, VK_IMAGE_ASPECT_COLOR_BIT }},
    {rhi::Format::R16G16B16_SINT, { VK_FORMAT_R16G16B16_SINT, VK_IMAGE_ASPECT_COLOR_BIT }},
    {rhi::Format::R16G16B16_SNORM, { VK_FORMAT_R16G16B16_SNORM, VK_IMAGE_ASPECT_COLOR_BIT }},
    {rhi::Format::R16G16B16_SSCALED, { VK_FORMAT_R16G16B16_SSCALED, VK_IMAGE_ASPECT_COLOR_BIT }},
    {rhi::Format::R16G16B16_UINT, { VK_FORMAT_R16G16B16_UINT, VK_IMAGE_ASPECT_COLOR_BIT }},
    {rhi::Format::R16G16B16_UNORM, { VK_FORMAT_R16G16B16_UNORM, VK_IMAGE_ASPECT_COLOR_BIT }},
    {rhi::Format::R16G16B16_USCALED, { VK_FORMAT_R16G16B16_USCALED, VK_IMAGE_ASPECT_COLOR_BIT }},
    {rhi::Format::R16G16_FLOAT, { VK_FORMAT_R16G16_SFLOAT, VK_IMAGE_ASPECT_COLOR_BIT }},
    {rhi::Format::R16G16_SINT, { VK_FORMAT_R16G16_SINT, VK_IMAGE_ASPECT_COLOR_BIT }},
    {rhi::Format::R16G16_SNORM, { VK_FORMAT_R16G16_SNORM, VK_IMAGE_ASPECT_COLOR_BIT }},
    {rhi::Format::R16G16_SSCALED, { VK_FORMAT_R16G16_SSCALED, VK_IMAGE_ASPECT_COLOR_BIT }},
    {rhi::Format::R16G16_UINT, { VK_FORMAT_R16G16_UINT, VK_IMAGE_ASPECT_COLOR_BIT }},
    {rhi::Format::R16G16_UNORM, { VK_FORMAT_R16G16_UNORM, VK_IMAGE_ASPECT_COLOR_BIT }},
    {rhi::Format::R16G16_USCALED, { VK_FORMAT_R16G16_USCALED, VK_IMAGE_ASPECT_COLOR_BIT }},
    {rhi::Format::R16_FLOAT, { VK_FORMAT_R16_SFLOAT, VK_IMAGE_ASPECT_COLOR_BIT }},
    {rhi::Format::R16_SINT, { VK_FORMAT_R16_SINT, VK_IMAGE_ASPECT_COLOR_BIT }},
    {rhi::Format::R16_SNORM, { VK_FORMAT_R16_SNORM, VK_IMAGE_ASPECT_COLOR_BIT }},
    {rhi::Format::R16_SSCALED, { VK_FORMAT_R16_SSCALED, VK_IMAGE_ASPECT_COLOR_BIT }},
    {rhi::Format::R16_UINT, { VK_FORMAT_R16_UINT, VK_IMAGE_ASPECT_COLOR_BIT }},
    {rhi::Format::R16_UNORM, { VK_FORMAT_R16_UNORM, VK_IMAGE_ASPECT_COLOR_BIT }},
    {rhi::Format::R16_USCALED, { VK_FORMAT_R16_USCALED, VK_IMAGE_ASPECT_COLOR_BIT }},
    {rhi::Format::R32G32B32A32_UNORM, { VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_ASPECT_COLOR_BIT } },
    {rhi::Format::R32G32B32A32_FLOAT, { VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_ASPECT_COLOR_BIT }},
    {rhi::Format::R32G32B32A32_SINT, { VK_FORMAT_R32G32B32A32_SINT, VK_IMAGE_ASPECT_COLOR_BIT }},
    {rhi::Format::R32G32B32A32_UINT, { VK_FORMAT_R32G32B32A32_UINT, VK_IMAGE_ASPECT_COLOR_BIT }},
    {rhi::Format::R32G32B32_FLOAT, { VK_FORMAT_R32G32B32_SFLOAT, VK_IMAGE_ASPECT_COLOR_BIT }},
    {rhi::Format::R32G32B32_SINT, { VK_FORMAT_R32G32B32_SINT, VK_IMAGE_ASPECT_COLOR_BIT }},
    {rhi::Format::R32G32B32_UINT, { VK_FORMAT_R32G32B32_UINT, VK_IMAGE_ASPECT_COLOR_BIT }},
    {rhi::Format::R32G32_FLOAT, { VK_FORMAT_R32G32_SFLOAT, VK_IMAGE_ASPECT_COLOR_BIT }},
    {rhi::Format::R32G32_SINT, { VK_FORMAT_R32G32_SINT, VK_IMAGE_ASPECT_COLOR_BIT }},
    {rhi::Format::R32G32_UINT, { VK_FORMAT_R32G32_UINT, VK_IMAGE_ASPECT_COLOR_BIT }},
    {rhi::Format::R32_FLOAT, { VK_FORMAT_R32_SFLOAT, VK_IMAGE_ASPECT_COLOR_BIT }},
    {rhi::Format::R32_SINT, { VK_FORMAT_R32_SINT, VK_IMAGE_ASPECT_COLOR_BIT }},
    {rhi::Format::R32_UINT, { VK_FORMAT_R32_UINT, VK_IMAGE_ASPECT_COLOR_BIT }},
    {rhi::Format::R4G4B4A4_UNORM, { VK_FORMAT_R4G4B4A4_UNORM_PACK16, VK_IMAGE_ASPECT_COLOR_BIT }},
    {rhi::Format::R5G5B5A1_UNORM, { VK_FORMAT_R5G5B5A1_UNORM_PACK16, VK_IMAGE_ASPECT_COLOR_BIT }},
    {rhi::Format::R5G6B5_UNORM, { VK_FORMAT_R5G6B5_UNORM_PACK16, VK_IMAGE_ASPECT_COLOR_BIT }},
    {rhi::Format::R8G8B8A8_SINT, { VK_FORMAT_R8G8B8A8_SINT, VK_IMAGE_ASPECT_COLOR_BIT }},
    {rhi::Format::R8G8B8A8_SNORM, { VK_FORMAT_R8G8B8A8_SNORM, VK_IMAGE_ASPECT_COLOR_BIT }},
    {rhi::Format::R8G8B8A8_SSCALED, { VK_FORMAT_R8G8B8A8_SSCALED, VK_IMAGE_ASPECT_COLOR_BIT }},
    {rhi::Format::R8G8B8A8_UINT, { VK_FORMAT_R8G8B8A8_UINT, VK_IMAGE_ASPECT_COLOR_BIT }},
    {rhi::Format::R8G8B8A8_UNORM, { VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT }},
    {rhi::Format::R8G8B8A8_UNORM_SRGB, { VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT }},
    {rhi::Format::R8G8B8A8_USCALED, { VK_FORMAT_R8G8B8A8_USCALED, VK_IMAGE_ASPECT_COLOR_BIT }},
    {rhi::Format::R8G8B8_SINT, { VK_FORMAT_R8G8B8_SINT, VK_IMAGE_ASPECT_COLOR_BIT }},
    {rhi::Format::R8G8B8_SNORM, { VK_FORMAT_R8G8B8_SNORM, VK_IMAGE_ASPECT_COLOR_BIT }},
    {rhi::Format::R8G8B8_SSCALED, { VK_FORMAT_R8G8B8_SSCALED, VK_IMAGE_ASPECT_COLOR_BIT }},
    {rhi::Format::R8G8B8_UINT, { VK_FORMAT_R8G8B8_UINT, VK_IMAGE_ASPECT_COLOR_BIT }},
    {rhi::Format::R8G8B8_UNORM_SRGB, { VK_FORMAT_R8G8B8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT }},
    {rhi::Format::R8G8B8_USCALED, { VK_FORMAT_R8G8B8_USCALED, VK_IMAGE_ASPECT_COLOR_BIT }},
    {rhi::Format::R8G8_SINT, { VK_FORMAT_R8G8_SINT, VK_IMAGE_ASPECT_COLOR_BIT }},
    {rhi::Format::R8G8_SNORM, { VK_FORMAT_R8G8_SNORM, VK_IMAGE_ASPECT_COLOR_BIT }},
    {rhi::Format::R8G8_SSCALED, { VK_FORMAT_R8G8_SSCALED, VK_IMAGE_ASPECT_COLOR_BIT }},
    {rhi::Format::R8G8_UINT, { VK_FORMAT_R8G8_UINT, VK_IMAGE_ASPECT_COLOR_BIT }},
    {rhi::Format::R8G8_UNORM, { VK_FORMAT_R8G8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT }},
    {rhi::Format::R8G8_UNORM_SRGB, { VK_FORMAT_R8G8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT }},
    {rhi::Format::R8G8_USCALED, { VK_FORMAT_R8G8_USCALED, VK_IMAGE_ASPECT_COLOR_BIT }},
    {rhi::Format::R8_SINT, { VK_FORMAT_R8_SINT, VK_IMAGE_ASPECT_COLOR_BIT }},
    {rhi::Format::R8_SNORM, { VK_FORMAT_R8_SNORM, VK_IMAGE_ASPECT_COLOR_BIT }},
    {rhi::Format::R8_SSCALED, { VK_FORMAT_R8_SSCALED, VK_IMAGE_ASPECT_COLOR_BIT }},
    {rhi::Format::R8_UINT, { VK_FORMAT_R8_UINT, VK_IMAGE_ASPECT_COLOR_BIT }},
    {rhi::Format::R8_UNORM, { VK_FORMAT_R8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT }},
    {rhi::Format::R8_UNORM_SRGB, { VK_FORMAT_R8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT }},
    {rhi::Format::R8_USCALED, { VK_FORMAT_R8_USCALED, VK_IMAGE_ASPECT_COLOR_BIT }},
    {rhi::Format::R9G9B9E5_SHAREDEXP, { VK_FORMAT_E5B9G9R9_UFLOAT_PACK32, VK_IMAGE_ASPECT_COLOR_BIT }},
    // Depth
    {rhi::Format::D16_UNORM, { VK_FORMAT_D16_UNORM, VK_IMAGE_ASPECT_DEPTH_BIT } },
    { rhi::Format::D24_UNORM_X8_UINT, { VK_FORMAT_X8_D24_UNORM_PACK32, VK_IMAGE_ASPECT_DEPTH_BIT }},
    { rhi::Format::D32_FLOAT, { VK_FORMAT_D32_SFLOAT, VK_IMAGE_ASPECT_DEPTH_BIT }},
    // Stencil
    { rhi::Format::S8_UINT, { VK_FORMAT_S8_UINT, VK_IMAGE_ASPECT_STENCIL_BIT }},
    // DS
    { rhi::Format::D24_UNORM_S8_UINT, { VK_FORMAT_D24_UNORM_S8_UINT, VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT }},
    { rhi::Format::D32_FLOAT_S8X24_UINT, { VK_FORMAT_D32_SFLOAT_S8_UINT, VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT }}
};

inline VkFormat convertToVkFormat(rhi::Format formatID)
{
    return kFormatMap[formatID].format;
}

inline VkImageAspectFlags getImageAspectMask(VkFormat format)
{
    switch (format)
    {
    case VK_FORMAT_D16_UNORM:
    case VK_FORMAT_X8_D24_UNORM_PACK32:
    case VK_FORMAT_D32_SFLOAT:
        return VK_IMAGE_ASPECT_DEPTH_BIT;
    case VK_FORMAT_S8_UINT:
        return VK_IMAGE_ASPECT_STENCIL_BIT;
    case VK_FORMAT_D24_UNORM_S8_UINT:
    case VK_FORMAT_D32_SFLOAT_S8_UINT:
        return VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
    default:
        return VK_IMAGE_ASPECT_COLOR_BIT;
    }
}



inline VkImageViewType getImageViewType(uint32_t width, uint32_t height, uint32_t depth)
{
    if (depth > 1)
    {
        return VK_IMAGE_VIEW_TYPE_3D;
    }
    else
    {
        return VK_IMAGE_VIEW_TYPE_2D;
    }
}

enum ResourceAccess
{
    Unused,
    ReadOnly,
    Write,
};

// This defines enum for VkPipelineStageFlagBits so that we can use it to compare and index into
// array.
enum class PipelineStage : uint16_t
{
    // Bellow are ordered based on Graphics Pipeline Stages
    TopOfPipe = 0,
    DrawIndirect = 1,
    VertexInput = 2,
    VertexShader = 3,
    GeometryShader = 4,
    TransformFeedback = 5,
    EarlyFragmentTest = 6,
    FragmentShader = 7,
    LateFragmentTest = 8,
    ColorAttachmentOutput = 9,

    // Compute specific pipeline Stage
    ComputeShader = 10,

    // Transfer specific pipeline Stage
    Transfer = 11,
    BottomOfPipe = 12,

    // Host specific pipeline stage
    Host = 13,

    InvalidEnum = 14,
    EnumCount = InvalidEnum,
};

struct ImageMemoryBarrierData
{
    char name[44];

    // The Vk layout corresponding to the ImageLayout key.
    VkImageLayout layout;

    // The stage in which the image is used (or Bottom/Top if not using any specific stage).  Unless
    // Bottom/Top (Bottom used for transition to and Top used for transition from), the two values
    // should match.
    VkPipelineStageFlags dstStageMask;
    VkPipelineStageFlags srcStageMask;
    // Access mask when transitioning into this layout.
    VkAccessFlags dstAccessMask;
    // Access mask when transitioning out from this layout.  Note that source access mask never
    // needs a READ bit, as WAR hazards don't need memory barriers (just execution barriers).
    VkAccessFlags srcAccessMask;
    // Read or write.
    ResourceAccess type;
    // CommandBufferHelper tracks an array of PipelineBarriers. This indicates which array element
    // this should be merged into. Right now we track individual barrier for every PipelineStage. If
    // layout has a single stage mask bit, we use that stage as index. If layout has multiple stage
    // mask bits, we pick the lowest stage as the index since it is the first stage that needs
    // barrier.
    PipelineStage barrierIndex;
};

constexpr VkPipelineStageFlags kPreFragmentStageFlags =
VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT |
VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT | VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT;

constexpr VkPipelineStageFlags kAllShadersPipelineStageFlags =
kPreFragmentStageFlags | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT |
VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;

constexpr VkPipelineStageFlags kAllDepthStencilPipelineStageFlags =
VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;

// clang-format off
static std::map<ImageLayout, ImageMemoryBarrierData> kImageMemoryBarrierData = {
    {
        ImageLayout::Undefined,
        ImageMemoryBarrierData{
            "Undefined",
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
            VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
            // Transition to: we don't expect to transition into Undefined.
            0,
            // Transition from: there's no data in the image to care about.
            0,
            ResourceAccess::ReadOnly,
            PipelineStage::InvalidEnum,
        },
    },
    {
        ImageLayout::ColorAttachment,
        ImageMemoryBarrierData{
            "ColorAttachment",
            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            // Transition to: all reads and writes must happen after barrier.
            VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
            // Transition from: all writes must finish before barrier.
            VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
            ResourceAccess::Write,
            PipelineStage::ColorAttachmentOutput,
        },
    },
    {
        ImageLayout::ColorAttachmentAndFragmentShaderRead,
        ImageMemoryBarrierData{
            "ColorAttachmentAndFragmentShaderRead",
            VK_IMAGE_LAYOUT_GENERAL,
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
            // Transition to: all reads and writes must happen after barrier.
            VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_SHADER_READ_BIT,
            // Transition from: all writes must finish before barrier.
            VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
            ResourceAccess::Write,
            PipelineStage::FragmentShader,
        },
    },
    {
        ImageLayout::ColorAttachmentAndAllShadersRead,
        ImageMemoryBarrierData{
            "ColorAttachmentAndAllShadersRead",
            VK_IMAGE_LAYOUT_GENERAL,
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | kAllShadersPipelineStageFlags,
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | kAllShadersPipelineStageFlags,
            // Transition to: all reads and writes must happen after barrier.
            VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_SHADER_READ_BIT,
            // Transition from: all writes must finish before barrier.
            VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
            ResourceAccess::Write,
            // In case of multiple destination stages, We barrier the earliest stage
            PipelineStage::VertexShader,
        },
    },
    {
        ImageLayout::DSAttachmentWriteAndFragmentShaderRead,
        ImageMemoryBarrierData{
            "DSAttachmentWriteAndFragmentShaderRead",
            VK_IMAGE_LAYOUT_GENERAL,
            kAllDepthStencilPipelineStageFlags | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
            kAllDepthStencilPipelineStageFlags | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
            // Transition to: all reads and writes must happen after barrier.
            VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_SHADER_READ_BIT,
            // Transition from: all writes must finish before barrier.
            VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
            ResourceAccess::Write,
            PipelineStage::FragmentShader,
        },
    },
    {
        ImageLayout::DSAttachmentWriteAndAllShadersRead,
        ImageMemoryBarrierData{
            "DSAttachmentWriteAndAllShadersRead",
            VK_IMAGE_LAYOUT_GENERAL,
            kAllDepthStencilPipelineStageFlags | kAllShadersPipelineStageFlags,
            kAllDepthStencilPipelineStageFlags | kAllShadersPipelineStageFlags,
            // Transition to: all reads and writes must happen after barrier.
            VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_SHADER_READ_BIT,
            // Transition from: all writes must finish before barrier.
            VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
            ResourceAccess::Write,
            // In case of multiple destination stages, We barrier the earliest stage
            PipelineStage::VertexShader,
        },
    },
    {
        ImageLayout::DSAttachmentReadAndFragmentShaderRead,
            ImageMemoryBarrierData{
            "DSAttachmentReadAndFragmentShaderRead",
            VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,
            VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | kAllDepthStencilPipelineStageFlags,
            VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | kAllDepthStencilPipelineStageFlags,
            // Transition to: all reads must happen after barrier.
            VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT,
            // Transition from: RAR and WAR don't need memory barrier.
            0,
            ResourceAccess::ReadOnly,
            PipelineStage::EarlyFragmentTest,
        },
    },
    {
        ImageLayout::DSAttachmentReadAndAllShadersRead,
            ImageMemoryBarrierData{
            "DSAttachmentReadAndAllShadersRead",
            VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,
            kAllShadersPipelineStageFlags | kAllDepthStencilPipelineStageFlags,
            kAllShadersPipelineStageFlags | kAllDepthStencilPipelineStageFlags,
            // Transition to: all reads must happen after barrier.
            VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT,
            // Transition from: RAR and WAR don't need memory barrier.
            0,
            ResourceAccess::ReadOnly,
            PipelineStage::VertexShader,
        },
    },
    {
        ImageLayout::DepthStencilAttachmentReadOnly,
            ImageMemoryBarrierData{
            "DepthStencilAttachmentReadOnly",
            VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,
            kAllDepthStencilPipelineStageFlags,
            kAllDepthStencilPipelineStageFlags,
            // Transition to: all reads must happen after barrier.
            VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT,
            // Transition from: RAR and WAR don't need memory barrier.
            0,
            ResourceAccess::ReadOnly,
            PipelineStage::EarlyFragmentTest,
        },
    },
    {
        ImageLayout::DepthStencilAttachment,
        ImageMemoryBarrierData{
            "DepthStencilAttachment",
            VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
            kAllDepthStencilPipelineStageFlags,
            kAllDepthStencilPipelineStageFlags,
            // Transition to: all reads and writes must happen after barrier.
            VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
            // Transition from: all writes must finish before barrier.
            VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
            ResourceAccess::Write,
            PipelineStage::EarlyFragmentTest,
        },
    },
    {
        ImageLayout::DepthStencilResolveAttachment,
        ImageMemoryBarrierData{
            "DepthStencilResolveAttachment",
            VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
            // Note: depth/stencil resolve uses color output stage and mask!
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            // Transition to: all reads and writes must happen after barrier.
            VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
            // Transition from: all writes must finish before barrier.
            VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
            ResourceAccess::Write,
            PipelineStage::ColorAttachmentOutput,
        },
    },
    {
        ImageLayout::Present,
        ImageMemoryBarrierData{
            "Present",
            VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
            VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
            VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
            // transition to: vkQueuePresentKHR automatically performs the appropriate memory barriers:
            //
            // > Any writes to memory backing the images referenced by the pImageIndices and
            // > pSwapchains members of pPresentInfo, that are available before vkQueuePresentKHR
            // > is executed, are automatically made visible to the read access performed by the
            // > presentation engine.
            0,
            // Transition from: RAR and WAR don't need memory barrier.
            0,
            ResourceAccess::ReadOnly,
            PipelineStage::BottomOfPipe,
        },
    },
    {
        ImageLayout::SharedPresent,
        ImageMemoryBarrierData{
            "SharedPresent",
            VK_IMAGE_LAYOUT_SHARED_PRESENT_KHR,
            VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
            VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
            // Transition to: all reads and writes must happen after barrier.
            VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT,
            // Transition from: all writes must finish before barrier.
            VK_ACCESS_MEMORY_WRITE_BIT,
            ResourceAccess::Write,
            PipelineStage::BottomOfPipe,
        },
    },
    {
        ImageLayout::ExternalPreInitialized,
        ImageMemoryBarrierData{
            "ExternalPreInitialized",
            VK_IMAGE_LAYOUT_PREINITIALIZED,
            VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
            VK_PIPELINE_STAGE_HOST_BIT | VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
            // Transition to: we don't expect to transition into PreInitialized.
            0,
            // Transition from: all writes must finish before barrier.
            VK_ACCESS_MEMORY_WRITE_BIT,
            ResourceAccess::ReadOnly,
            PipelineStage::InvalidEnum,
        },
    },
    {
        ImageLayout::ExternalShadersReadOnly,
        ImageMemoryBarrierData{
            "ExternalShadersReadOnly",
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
            VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
            // Transition to: all reads must happen after barrier.
            VK_ACCESS_SHADER_READ_BIT,
            // Transition from: RAR and WAR don't need memory barrier.
            0,
            ResourceAccess::ReadOnly,
            // In case of multiple destination stages, We barrier the earliest stage
            PipelineStage::TopOfPipe,
        },
    },
    {
        ImageLayout::ExternalShadersWrite,
        ImageMemoryBarrierData{
            "ExternalShadersWrite",
            VK_IMAGE_LAYOUT_GENERAL,
            VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
            VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
            // Transition to: all reads and writes must happen after barrier.
            VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT,
            // Transition from: all writes must finish before barrier.
            VK_ACCESS_SHADER_WRITE_BIT,
            ResourceAccess::Write,
            // In case of multiple destination stages, We barrier the earliest stage
            PipelineStage::TopOfPipe,
        },
    },
    {
        ImageLayout::TransferSrc,
        ImageMemoryBarrierData{
            "TransferSrc",
            VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            // Transition to: all reads must happen after barrier.
            VK_ACCESS_TRANSFER_READ_BIT,
            // Transition from: RAR and WAR don't need memory barrier.
            0,
            ResourceAccess::ReadOnly,
            PipelineStage::Transfer,
        },
    },
    {
        ImageLayout::TransferDst,
        ImageMemoryBarrierData{
            "TransferDst",
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            // Transition to: all writes must happen after barrier.
            VK_ACCESS_TRANSFER_WRITE_BIT,
            // Transition from: all writes must finish before barrier.
            VK_ACCESS_TRANSFER_WRITE_BIT,
            ResourceAccess::Write,
            PipelineStage::Transfer,
        },
    },
    {
        ImageLayout::VertexShaderReadOnly,
        ImageMemoryBarrierData{
            "VertexShaderReadOnly",
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            VK_PIPELINE_STAGE_VERTEX_SHADER_BIT,
            VK_PIPELINE_STAGE_VERTEX_SHADER_BIT,
            // Transition to: all reads must happen after barrier.
            VK_ACCESS_SHADER_READ_BIT,
            // Transition from: RAR and WAR don't need memory barrier.
            0,
            ResourceAccess::ReadOnly,
            PipelineStage::VertexShader,
        },
    },
    {
        ImageLayout::VertexShaderWrite,
        ImageMemoryBarrierData{
            "VertexShaderWrite",
            VK_IMAGE_LAYOUT_GENERAL,
            VK_PIPELINE_STAGE_VERTEX_SHADER_BIT,
            VK_PIPELINE_STAGE_VERTEX_SHADER_BIT,
            // Transition to: all reads and writes must happen after barrier.
            VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT,
            // Transition from: all writes must finish before barrier.
            VK_ACCESS_SHADER_WRITE_BIT,
            ResourceAccess::Write,
            PipelineStage::VertexShader,
        },
    },
    {
        ImageLayout::PreFragmentShadersReadOnly,
        ImageMemoryBarrierData{
            "PreFragmentShadersReadOnly",
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            kPreFragmentStageFlags,
            kPreFragmentStageFlags,
            // Transition to: all reads must happen after barrier.
            VK_ACCESS_SHADER_READ_BIT,
            // Transition from: RAR and WAR don't need memory barrier.
            0,
            ResourceAccess::ReadOnly,
            // In case of multiple destination stages, We barrier the earliest stage
            PipelineStage::VertexShader,
        },
    },
    {
        ImageLayout::PreFragmentShadersWrite,
        ImageMemoryBarrierData{
            "PreFragmentShadersWrite",
            VK_IMAGE_LAYOUT_GENERAL,
            kPreFragmentStageFlags,
            kPreFragmentStageFlags,
            // Transition to: all reads and writes must happen after barrier.
            VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT,
            // Transition from: all writes must finish before barrier.
            VK_ACCESS_SHADER_WRITE_BIT,
            ResourceAccess::Write,
            // In case of multiple destination stages, We barrier the earliest stage
            PipelineStage::VertexShader,
        },
    },
    {
        ImageLayout::FragmentShaderReadOnly,
        ImageMemoryBarrierData{
            "FragmentShaderReadOnly",
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
            VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
            // Transition to: all reads must happen after barrier.
            VK_ACCESS_SHADER_READ_BIT,
            // Transition from: RAR and WAR don't need memory barrier.
            0,
            ResourceAccess::ReadOnly,
            PipelineStage::FragmentShader,
        },
    },
    {
        ImageLayout::FragmentShaderWrite,
        ImageMemoryBarrierData{
            "FragmentShaderWrite",
            VK_IMAGE_LAYOUT_GENERAL,
            VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
            VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
            // Transition to: all reads and writes must happen after barrier.
            VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT,
            // Transition from: all writes must finish before barrier.
            VK_ACCESS_SHADER_WRITE_BIT,
            ResourceAccess::Write,
            PipelineStage::FragmentShader,
        },
    },
    {
        ImageLayout::ComputeShaderReadOnly,
        ImageMemoryBarrierData{
            "ComputeShaderReadOnly",
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
            VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
            // Transition to: all reads must happen after barrier.
            VK_ACCESS_SHADER_READ_BIT,
            // Transition from: RAR and WAR don't need memory barrier.
            0,
            ResourceAccess::ReadOnly,
            PipelineStage::ComputeShader,
        },
    },
    {
        ImageLayout::ComputeShaderWrite,
        ImageMemoryBarrierData{
            "ComputeShaderWrite",
            VK_IMAGE_LAYOUT_GENERAL,
            VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
            VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
            // Transition to: all reads and writes must happen after barrier.
            VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT,
            // Transition from: all writes must finish before barrier.
            VK_ACCESS_SHADER_WRITE_BIT,
            ResourceAccess::Write,
            PipelineStage::ComputeShader,
        },
    },
    {
        ImageLayout::AllGraphicsShadersReadOnly,
        ImageMemoryBarrierData{
            "AllGraphicsShadersReadOnly",
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            kAllShadersPipelineStageFlags,
            kAllShadersPipelineStageFlags,
            // Transition to: all reads must happen after barrier.
            VK_ACCESS_SHADER_READ_BIT,
            // Transition from: RAR and WAR don't need memory barrier.
            0,
            ResourceAccess::ReadOnly,
            // In case of multiple destination stages, We barrier the earliest stage
            PipelineStage::VertexShader,
        },
    },
    {
        ImageLayout::AllGraphicsShadersWrite,
        ImageMemoryBarrierData{
            "AllGraphicsShadersWrite",
            VK_IMAGE_LAYOUT_GENERAL,
            kAllShadersPipelineStageFlags,
            kAllShadersPipelineStageFlags,
            // Transition to: all reads and writes must happen after barrier.
            VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT,
            // Transition from: all writes must finish before barrier.
            VK_ACCESS_SHADER_WRITE_BIT,
            ResourceAccess::Write,
            // In case of multiple destination stages, We barrier the earliest stage
            PipelineStage::VertexShader,
        },
    },
};

inline VkImageLayout convertToVkImageLayout(ImageLayout imageLayout)
{
    return kImageMemoryBarrierData[imageLayout].layout;
}

/*
    VkCullModeFlags                            cullMode;
    VkFrontFace                                frontFace;
*/
inline VkFrontFace convertToVkFrontFace(rhi::FrontFace frontFace)
{
    switch (frontFace)
    {
    case rhi::FrontFace::COUNTER_CLOCKWISE:
        return VK_FRONT_FACE_COUNTER_CLOCKWISE;
    case rhi::FrontFace::CLOCKWISE:
        return VK_FRONT_FACE_CLOCKWISE;
    default:
        UNREACHABLE();
        return VK_FRONT_FACE_COUNTER_CLOCKWISE;
    }

}

inline VkCullModeFlags convertToVkCullMode(rhi::CullMode cullMode)
{
    switch (cullMode)
    {
    case rhi::CullMode::NONE:
        return VK_CULL_MODE_NONE;
    case rhi::CullMode::FRONT_BIT:
        return VK_CULL_MODE_FRONT_BIT;
    case rhi::CullMode::BACK_BIT:
        return VK_CULL_MODE_BACK_BIT;
    case rhi::CullMode::FRONT_AND_BACK:
        return VK_CULL_MODE_FRONT_AND_BACK;
    default:
        UNREACHABLE();
        return VK_CULL_MODE_NONE;
    }
}

inline VkPolygonMode convertToVkPolygonMode(rhi::PolygonMode polygonMode)
{
    switch (polygonMode)
    {
    case rhi::PolygonMode::FILL:
        return VK_POLYGON_MODE_FILL;
    case rhi::PolygonMode::LINE:
        return VK_POLYGON_MODE_LINE;
    case rhi::PolygonMode::POINT:
        return VK_POLYGON_MODE_POINT;
    default:
        UNREACHABLE();
        return VK_POLYGON_MODE_FILL;
    }
}

inline VkIndexType convertToVkIndexType(rhi::IndexSize indexSize)
{
    switch (indexSize)
    {
    case rhi::IndexSize::UINT32:
        return VK_INDEX_TYPE_UINT32;
    case rhi::IndexSize::UINT16:
        return VK_INDEX_TYPE_UINT16;
    default:
        UNREACHABLE();
        return VK_INDEX_TYPE_NONE_KHR;
    }
}

inline VkPrimitiveTopology convertToVkPrimitiveTopology(rhi::Topology topology)
{
    switch (topology)
    {
    case rhi::Topology::LINE_LIST:
        return VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
    case rhi::Topology::LINE_LIST_WITH_ADJACENCY:
        return VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY;
    case rhi::Topology::LINE_STRIP:
        return VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
    case rhi::Topology::LINE_STRIP_WITH_ADJACENCY:
        return VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_LINE_STRIP_WITH_ADJACENCY;
    case rhi::Topology::PATCH_LIST:
        return VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_PATCH_LIST;
    case rhi::Topology::POINT_LIST:
        return VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
    case rhi::Topology::TRIANGLE_FAN:
        return VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN;
    case rhi::Topology::TRIANGLE_LIST:
        return VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    case rhi::Topology::TRIANGLE_LIST_WITH_ADJACENCY:
        return VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY;
    case rhi::Topology::TRIANGLE_STRIP:
        return VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
    case rhi::Topology::TRIANGLE_STRIP_WITH_ADJACENCY:
        return VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_ADJACENCY;
    default:
        UNREACHABLE();
        return VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    }
}

inline VkBufferUsageFlags convertToVkBufferUsageFlag(rhi::BufferUsageFlags rhiBufferUsageFlags)
{
    VkBufferUsageFlags bufferUsageFlags = 0;

    if ((rhiBufferUsageFlags & rhi::BufferUsage::BUFFER_TRANSFER_SRC) != 0)
    {
        bufferUsageFlags |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    }

    if ((rhiBufferUsageFlags & rhi::BufferUsage::BUFFER_TRANSFER_DST) != 0)
    {
        bufferUsageFlags |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    }

    if ((rhiBufferUsageFlags & rhi::BufferUsage::BUFFER_UNIFORM_TEXEL_BUFFER) != 0)
    {
        bufferUsageFlags |= VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT;
    }

    if ((rhiBufferUsageFlags & rhi::BufferUsage::BUFFER_STORAGE_BUFFER) != 0)
    {
        bufferUsageFlags |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    }

    if ((rhiBufferUsageFlags & rhi::BufferUsage::BUFFER_VERTEX_BUFFER) != 0)
    {
        bufferUsageFlags |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    }

    if ((rhiBufferUsageFlags & rhi::BufferUsage::BUFFER_INDEX_BUFFER) != 0)
    {
        bufferUsageFlags |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    }

    if ((rhiBufferUsageFlags & rhi::BufferUsage::BUFFER_INDIRECT_BUFFER) != 0)
    {
        bufferUsageFlags |= VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
    }

    if ((rhiBufferUsageFlags & rhi::BufferUsage::BUFFER_SHADER_DEVICE_ADDRESS) != 0)
    {
        bufferUsageFlags |= VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
    }

    if ((rhiBufferUsageFlags & rhi::BufferUsage::BUFFER_TRANSFORM_FEEDBACK_BUFFER) != 0)
    {
        bufferUsageFlags |= VK_BUFFER_USAGE_TRANSFORM_FEEDBACK_BUFFER_BIT_EXT;
    }

    if ((rhiBufferUsageFlags & rhi::BufferUsage::BUFFER_TRANSFORM_FEEDBACK_COUNTER_BUFFER) != 0)
    {
        bufferUsageFlags |= VK_BUFFER_USAGE_TRANSFORM_FEEDBACK_COUNTER_BUFFER_BIT_EXT;
    }

    if ((rhiBufferUsageFlags & rhi::BufferUsage::BUFFER_CONDITIONAL_RENDERING) != 0)
    {
        bufferUsageFlags |= VK_BUFFER_USAGE_CONDITIONAL_RENDERING_BIT_EXT;
    }

    if ((rhiBufferUsageFlags & rhi::BufferUsage::BUFFER_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY) != 0)
    {
        bufferUsageFlags |= VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR;
    }

    if ((rhiBufferUsageFlags & rhi::BufferUsage::BUFFER_ACCELERATION_STRUCTURE_STORAGE) != 0)
    {
        bufferUsageFlags |= VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR;
    }

    if ((rhiBufferUsageFlags & rhi::BufferUsage::BUFFER_SHADER_BINDING_TABLE) != 0)
    {
        bufferUsageFlags |= VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR;
    }

    return bufferUsageFlags;
}

} // namespace vk