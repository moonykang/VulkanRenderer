#include "vulkan/extension.h"

namespace vk
{
#define GET_INSTANCE_PROC(instance, func) func = reinterpret_cast<PFN_##func>(vkGetInstanceProcAddr(instance, #func))
#define GET_DEVICE_PROC(device, func) func = reinterpret_cast<PFN_##func>(vkGetDeviceProcAddr(device, #func))

Extension::Extension(const char* extensionName)
    : extensionName(extensionName)
    , support(false)
{
}

void Extension::check(std::vector<VkExtensionProperties>& supportedExtensions)
{
    for (auto& supportedExtension : supportedExtensions)
    {
        if (strcmp(supportedExtension.extensionName, extensionName) == 0)
        {
            support = true;
            break;
        }
    }
}

void Extension::add(std::vector<const char*>& requestedExtensions)
{
    if (support)
    {
        requestedExtensions.push_back(extensionName);
    }
}

InstanceExtension::InstanceExtension(const char* extensionName)
    : Extension(extensionName)
{
}

// VK_KHR_get_physical_device_properties2
PFN_vkGetPhysicalDeviceProperties2KHR vkGetPhysicalDeviceProperties2KHR;
PhysicalDeviceProperties2Extension::PhysicalDeviceProperties2Extension()
    : InstanceExtension(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)
{

}

void PhysicalDeviceProperties2Extension::fetch(VkInstance instance)
{
    if (support)
    {
        GET_INSTANCE_PROC(instance, vkGetPhysicalDeviceProperties2KHR);
    }
}

DeviceExtension::DeviceExtension(const char* extensionName)
    : Extension(extensionName)
{
}

InstanceExtension* ExtensionFactory::createInstanceExtension(ExtensionName extensionName)
{
    switch (extensionName)
    {
    case ExtensionName::PhysicalDeviceProperties2Extension:
        return new PhysicalDeviceProperties2Extension();
    default:
        UNREACHABLE();
        return nullptr;
    }
}

DeviceExtension* ExtensionFactory::createDeviceExtension(ExtensionName extensionName)
{
    switch (extensionName)
    {
    case ExtensionName::SwapchainExtension:
        return new SwapchainExtension();
    case ExtensionName::DebugMarkerExtension:
        return new DebugMarkerExtension();
    case ExtensionName::AccelerationStructureExtension:
        return new AccelerationStructureExtension();
    case ExtensionName::RayTracingPipelineExtension:
        return new RayTracingPipelineExtension();
    case ExtensionName::DeviceAddressExtension:
        return new DeviceAddressExtension();
    case ExtensionName::DeferredHostOperationsExtension:
        return new DeferredHostOperationsExtension();
    case ExtensionName::RayQuery:
        return new RayQueryExtension();
    case ExtensionName::DescriptorIndexing:
        return new DescriptorIndexingExtension();
    case ExtensionName::Spirv_1_4:
        return new Spirv_1_4_Extension();
    default:
        UNREACHABLE();
        return nullptr;
    }
}

// VK_KHR_swapchain
SwapchainExtension::SwapchainExtension()
    : DeviceExtension(VK_KHR_SWAPCHAIN_EXTENSION_NAME)
{
}
// VK_EXT_debug_marker
DebugMarkerExtension::DebugMarkerExtension()
    : DeviceExtension(VK_EXT_DEBUG_MARKER_EXTENSION_NAME)
{
}

// VK_KHR_acceleration_structure
PFN_vkCreateAccelerationStructureKHR vkCreateAccelerationStructureKHR;
PFN_vkDestroyAccelerationStructureKHR vkDestroyAccelerationStructureKHR;
PFN_vkGetAccelerationStructureBuildSizesKHR vkGetAccelerationStructureBuildSizesKHR;
PFN_vkGetAccelerationStructureDeviceAddressKHR vkGetAccelerationStructureDeviceAddressKHR;
PFN_vkBuildAccelerationStructuresKHR vkBuildAccelerationStructuresKHR;
PFN_vkCmdBuildAccelerationStructuresKHR vkCmdBuildAccelerationStructuresKHR;
PFN_vkGetRayTracingShaderGroupHandlesKHR vkGetRayTracingShaderGroupHandlesKHR;

AccelerationStructureExtension::AccelerationStructureExtension()
    : DeviceExtension(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME)
    , accelerationStructureFeatures({})
    , physicalDeviceAccelerationStructureProperties({})
{
}

void AccelerationStructureExtension::feature(void** &chain)
{
    if (support)
    {
        accelerationStructureFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR;
        accelerationStructureFeatures.accelerationStructure = true;
        *chain = &accelerationStructureFeatures;
        chain = &accelerationStructureFeatures.pNext;
    }
}

void AccelerationStructureExtension::fetch(VkDevice device)
{
    if (support)
    {
        GET_DEVICE_PROC(device, vkCreateAccelerationStructureKHR);
        GET_DEVICE_PROC(device, vkDestroyAccelerationStructureKHR);
        GET_DEVICE_PROC(device, vkGetAccelerationStructureBuildSizesKHR);
        GET_DEVICE_PROC(device, vkGetAccelerationStructureDeviceAddressKHR);
        GET_DEVICE_PROC(device, vkBuildAccelerationStructuresKHR);
        GET_DEVICE_PROC(device, vkCmdBuildAccelerationStructuresKHR);
        GET_DEVICE_PROC(device, vkGetRayTracingShaderGroupHandlesKHR);
    }
}

void AccelerationStructureExtension::property(std::map<VkStructureType, void*>& propertyMap, void** &chain)
{
    if (support)
    {
        propertyMap[VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_PROPERTIES_KHR] = &physicalDeviceAccelerationStructureProperties;

        physicalDeviceAccelerationStructureProperties = {};
        physicalDeviceAccelerationStructureProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_PROPERTIES_KHR;

        *chain = &physicalDeviceAccelerationStructureProperties;
        chain = &physicalDeviceAccelerationStructureProperties.pNext;
    }
}

// VK_KHR_ray_tracing_pipeline
PFN_vkCmdTraceRaysKHR vkCmdTraceRaysKHR;
PFN_vkCreateRayTracingPipelinesKHR vkCreateRayTracingPipelinesKHR;

RayTracingPipelineExtension::RayTracingPipelineExtension()
    : DeviceExtension(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME)
    , rayTracingFeatures()
    , physicalDeviceRayTracingPipelineProperties()
{
}

void RayTracingPipelineExtension::feature(void** &chain)
{
    if (support)
    {
        rayTracingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR;
        rayTracingFeatures.rayTracingPipeline = true;
        *chain = &rayTracingFeatures;
        chain = &rayTracingFeatures.pNext;
    }
}

void RayTracingPipelineExtension::fetch(VkDevice device)
{
    if (support)
    {
        GET_DEVICE_PROC(device, vkCmdTraceRaysKHR);
        GET_DEVICE_PROC(device, vkCreateRayTracingPipelinesKHR);
    }
}

void RayTracingPipelineExtension::property(std::map<VkStructureType, void*>& propertyMap, void** &chain)
{
    if (support)
    {
        propertyMap[VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR] = &physicalDeviceRayTracingPipelineProperties;
        physicalDeviceRayTracingPipelineProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR;

        *chain = &physicalDeviceRayTracingPipelineProperties;
        chain = &physicalDeviceRayTracingPipelineProperties.pNext;
    }
}

// VK_KHR_buffer_device_address
PFN_vkGetBufferDeviceAddressKHR vkGetBufferDeviceAddressKHR;

DeviceAddressExtension::DeviceAddressExtension()
    : DeviceExtension(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME)
    , bufferDeviceAddressFeatures()
{
}

void DeviceAddressExtension::feature(void** &chain)
{
    if (support)
    {
        bufferDeviceAddressFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES;
        bufferDeviceAddressFeatures.bufferDeviceAddress = true;
        *chain = &bufferDeviceAddressFeatures;
        chain = &bufferDeviceAddressFeatures.pNext;
    }
}

void DeviceAddressExtension::fetch(VkDevice device)
{
    if (support)
    {
        GET_DEVICE_PROC(device, vkGetBufferDeviceAddressKHR);
    }
}

PFN_vkCreateDeferredOperationKHR vkCreateDeferredOperationKHR;
PFN_vkDestroyDeferredOperationKHR vkDestroyDeferredOperationKHR;
PFN_vkGetDeferredOperationMaxConcurrencyKHR vkGetDeferredOperationMaxConcurrencyKHR;
PFN_vkGetDeferredOperationResultKHR vkGetDeferredOperationResultKHR;
PFN_vkDeferredOperationJoinKHR vkDeferredOperationJoinKHR;

DeferredHostOperationsExtension::DeferredHostOperationsExtension()
    : DeviceExtension(VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME)
{
}

void DeferredHostOperationsExtension::fetch(VkDevice device)
{
    if (support)
    {
        GET_DEVICE_PROC(device, vkCreateDeferredOperationKHR);
        GET_DEVICE_PROC(device, vkDestroyDeferredOperationKHR);
        GET_DEVICE_PROC(device, vkGetDeferredOperationMaxConcurrencyKHR);
        GET_DEVICE_PROC(device, vkGetDeferredOperationResultKHR);
        GET_DEVICE_PROC(device, vkDeferredOperationJoinKHR);
    }
}

// VK_KHR_ray_query
RayQueryExtension::RayQueryExtension()
    : DeviceExtension(VK_KHR_RAY_QUERY_EXTENSION_NAME)
    , rayQueryFeatures()
{

}

void RayQueryExtension::feature(void**& chain)
{
    if (support)
    {
        rayQueryFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_QUERY_FEATURES_KHR;
        rayQueryFeatures.rayQuery = true;
        *chain = &rayQueryFeatures;
        chain = &rayQueryFeatures.pNext;
    }
}

DescriptorIndexingExtension::DescriptorIndexingExtension()
    : DeviceExtension(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME)
    , descriptorIndexingFeatures()
    , descriptorIndexingProperties()
{
}

void DescriptorIndexingExtension::feature(void**& chain)
{
    if (support)
    {
        descriptorIndexingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES;
        /*
        descriptorIndexingFeatures.shaderInputAttachmentArrayDynamicIndexing = true;
        descriptorIndexingFeatures.shaderUniformTexelBufferArrayDynamicIndexing = true;
        descriptorIndexingFeatures.shaderStorageTexelBufferArrayDynamicIndexing = true;
        descriptorIndexingFeatures.shaderUniformBufferArrayNonUniformIndexing = true;
        descriptorIndexingFeatures.shaderSampledImageArrayNonUniformIndexing = true;
        descriptorIndexingFeatures.shaderStorageBufferArrayNonUniformIndexing = true;
        descriptorIndexingFeatures.shaderStorageImageArrayNonUniformIndexing = true;
        descriptorIndexingFeatures.shaderInputAttachmentArrayNonUniformIndexing = true;
        descriptorIndexingFeatures.shaderUniformTexelBufferArrayNonUniformIndexing = true;
        descriptorIndexingFeatures.shaderStorageTexelBufferArrayNonUniformIndexing = true;
        descriptorIndexingFeatures.descriptorBindingUniformBufferUpdateAfterBind = true;
        descriptorIndexingFeatures.descriptorBindingSampledImageUpdateAfterBind = true;
        descriptorIndexingFeatures.descriptorBindingStorageImageUpdateAfterBind = true;
        descriptorIndexingFeatures.descriptorBindingStorageBufferUpdateAfterBind = true;
        descriptorIndexingFeatures.descriptorBindingUniformTexelBufferUpdateAfterBind = true;
        descriptorIndexingFeatures.descriptorBindingStorageTexelBufferUpdateAfterBind = true;
        descriptorIndexingFeatures.descriptorBindingUpdateUnusedWhilePending = true;
        descriptorIndexingFeatures.descriptorBindingPartiallyBound = true;
        descriptorIndexingFeatures.descriptorBindingVariableDescriptorCount = true;
        descriptorIndexingFeatures.runtimeDescriptorArray = true;
        */
        *chain = &descriptorIndexingFeatures;
        chain = &descriptorIndexingFeatures.pNext;
    }
}

void DescriptorIndexingExtension::property(std::map<VkStructureType, void*>& propertyMap, void**& chain)
{
    if (support)
    {
        propertyMap[VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_PROPERTIES_EXT] = &descriptorIndexingProperties;
        descriptorIndexingProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_PROPERTIES_EXT;

        *chain = &descriptorIndexingProperties;
        chain = &descriptorIndexingProperties.pNext;
    }
}

Spirv_1_4_Extension::Spirv_1_4_Extension()
    : DeviceExtension(VK_KHR_SPIRV_1_4_EXTENSION_NAME)
{

}
}