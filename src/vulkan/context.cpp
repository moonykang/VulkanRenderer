#include <cstring>
#include <string>
#include <vector>

#include "platform/window.h"
#include "vulkan/context.h"
#include "vulkan/debug.h"
#include "vulkan/surface.h"
#include "vulkan/commandBufferManager.h"
#include "vulkan/queue.h"
#include "vulkan/extension.h"
#include "vulkan/buffer.h"

namespace vk
{
Context::Context()
    : surface(nullptr)
    , commandBufferManager(nullptr)
    , queue(nullptr)
    , descriptorPool(nullptr)
    , queueFamilyIndex(0)
    , physicalDeviceProperties()
    , physicalDeviceFeatures2()
    , physicalDeviceProperties2()
{

}

bool Context::init(platform::Window* window)
{
#if PLATFORM_ANDROID || USE_WIN_VULKAN_WRAPPER
    if (!InitVulkan())
    {
        LOGE("Failed to load vulkan");
        return false;
    }
#endif

	initInstance();

    if (surface == nullptr)
    {
        surface = new Surface();
    }

    if (commandBufferManager == nullptr)
    {
        commandBufferManager = new CommandBufferManager();
    }

    if (queue == nullptr)
    {
        queue = new Queue();
    }

    if (descriptorPool == nullptr)
    {
        descriptorPool = new DescriptorPool();
    }

    initPhysicalDevice();
    surface->initSurface(instance.getHandle(), window);
    initLogicalDevice();
    surface->initSwapchain(physicalDevice.getHandle(), device.getHandle());

    descriptorPool->init(device.getHandle());

    renderTargetWidth = surface->getSurfaceSize().width;
    renderTargetHeight = surface->getSurfaceSize().height;

	return true;
}

bool Context::terminate()
{
    queue->waitIdle();

    if (descriptorPool != nullptr)
    {
        descriptorPool->destroy(device.getHandle());
        delete descriptorPool;
        descriptorPool = nullptr;
    }

    if (queue != nullptr)
    {
        queue->destroy(device.getHandle());
        delete queue;
        queue = nullptr;
    }

    if (commandBufferManager != nullptr)
    {
        commandBufferManager->destory(device.getHandle());
        delete commandBufferManager;
        commandBufferManager = nullptr;
    }

    if (surface != nullptr)
    {
        surface->destroySwapchain(device.getHandle());
    }

    for (auto& instanceExtension : instanceExtensions)
    {
        delete instanceExtension;
    }
    instanceExtensions.clear();

    for (auto& deviceExtension : deviceExtensions)
    {
        delete deviceExtension;
    }
    deviceExtensions.clear();
    
    pipelineCache.destroy(device.getHandle());
    device.destroy();
    physicalDevice.release();

    if (surface != nullptr)
    {
        surface->destroySurface(instance.getHandle());
        delete surface;
        surface = nullptr;
    }

    if (enableValidationLayer)
    {
        freeDebugCallback(instance.getHandle());
    }
    instance.destroy();
    return true;
}

bool Context::initInstance()
{
    VkInstanceCreateInfo instanceCreateInfo = {};

    VkApplicationInfo applicationInfo = {};
    applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    applicationInfo.pApplicationName = "dev.moon.littlerenderer";
    applicationInfo.applicationVersion = VK_MAKE_VERSION(0, 0, 1);
    applicationInfo.pEngineName = "dev.moon.littlerenderer";
    applicationInfo.engineVersion = VK_MAKE_VERSION(0, 0, 1);
#if VK_USE_PLATFORM_WIN32_KHR
    applicationInfo.apiVersion = VK_MAKE_API_VERSION(0, 1, 2, 0);
#else    
    applicationInfo.apiVersion = VK_API_VERSION_1_2;
#endif

    std::vector<const char*> requestedExtensions;

    instanceExtensions.push_back(ExtensionFactory::createInstanceExtension(ExtensionName::PhysicalDeviceProperties2Extension));

    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
    std::vector<VkExtensionProperties> supportedExtensions(extensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, supportedExtensions.data());

    for (auto& extension : instanceExtensions)
    {
        extension->check(supportedExtensions);
        extension->add(requestedExtensions);
    }

    for (auto &extension : supportedExtensions)
    {
        bool notExist = true;

        // TODO
        for (auto requestedExtension : requestedExtensions)
        {
            if (std::string(requestedExtension) == std::string(extension.extensionName))
            {
                notExist = false;
            }
        }

        if (notExist)
        {
            requestedExtensions.push_back(extension.extensionName);
            LOGD("%s", extension.extensionName);
        }
    }

    if (enableValidationLayer)
    {
        LOGD("Finding layers...");
        const char *validationLayerName = "VK_LAYER_KHRONOS_validation";
        {
            uint32_t instanceLayerCount;
            vkEnumerateInstanceLayerProperties(&instanceLayerCount, nullptr);
            std::vector<VkLayerProperties> instanceLayerProperties(instanceLayerCount);
            vkEnumerateInstanceLayerProperties(&instanceLayerCount, instanceLayerProperties.data());
            bool validationLayerPresent = false;
            for (VkLayerProperties layer : instanceLayerProperties)
            {
                LOGD("Layers : %s", layer.layerName);

                if (strcmp(layer.layerName, validationLayerName) == 0)
                {
                    validationLayerPresent = true;
                    break;
                }
            }
            if (validationLayerPresent)
            {
                instanceCreateInfo.ppEnabledLayerNames = &validationLayerName;
                instanceCreateInfo.enabledLayerCount = 1;
            }
            else
            {
                LOGE("Validation layer VK_LAYER_KHRONOS_validation not present, "
                     "validation is disabled");
            }
        }
    }

    instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceCreateInfo.pApplicationInfo = &applicationInfo;
    instanceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(requestedExtensions.size());
    instanceCreateInfo.ppEnabledExtensionNames = requestedExtensions.data();

    VKCALL(instance.init(instanceCreateInfo));

    for (auto& extension : instanceExtensions)
    {
        extension->fetch(instance.getHandle());
    }

    LOGD("Done to create instance");

#if !PLATFORM_ANDROID
    if (enableValidationLayer)
    {
        // The report flags determine what type of messages for the layers will
        // be displayed For validating (debugging) an application the error and
        // warning bits should suffice
        VkDebugReportFlagsEXT debugReportFlags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
        setupDebugging(instance.getHandle(), debugReportFlags, VK_NULL_HANDLE);
    }
#endif
    return true;
}

bool Context::initPhysicalDevice()
{
    // Physical device
    uint32_t gpuCount = 0;
    // Get number of available physical devices
    VKCALL(vkEnumeratePhysicalDevices(instance.getHandle(), &gpuCount, nullptr));

    if (gpuCount == 0)
    {
        LOGE("No device with Vulkan support found");
        return false;
    }
    // Enumerate devices
    std::vector<VkPhysicalDevice> physicalDevices(gpuCount);
    VKCALL(vkEnumeratePhysicalDevices(instance.getHandle(), &gpuCount, physicalDevices.data()));

    uint32_t selectedDevice = 0;

#if !PLATFORM_ANDROID
    for (uint32_t i = 0; i < gpuCount; i++) {
        vkGetPhysicalDeviceProperties(physicalDevices[i], &physicalDeviceProperties);
        LOGD("Device [%d]: %s", i, physicalDeviceProperties.deviceName);
        LOGD("Type: %s", debug::physicalDeviceTypeString(physicalDeviceProperties.deviceType).c_str());
    }
#endif
    
    physicalDevice.setHandle(physicalDevices[selectedDevice]);
    vkGetPhysicalDeviceProperties(physicalDevice.getHandle(), &physicalDeviceProperties);
    vkGetPhysicalDeviceFeatures(physicalDevice.getHandle(), &physicalDeviceFeatures2.features);
    
    gpuName = std::string(physicalDeviceProperties.deviceName);

    LOGD("Physical Device %s", physicalDeviceProperties.deviceName);
    LOGD("Tessellation shader %d", physicalDeviceFeatures2.features.tessellationShader);
    LOGD("Sampler Anisotropy %d", (int)physicalDeviceFeatures2.features.samplerAnisotropy);


    return true;
}

bool Context::initLogicalDevice()
{
    uint32_t graphicsQueueIndex = 0;

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos{};
    queueCreateInfos.push_back(getQueueCreateInfo(VK_QUEUE_GRAPHICS_BIT, &graphicsQueueIndex));

    // Get list of supported extensions
    uint32_t extensionCount = 0;
    std::vector<VkExtensionProperties> supportedExtensions;
    std::vector<const char*> requestedExtensions;
    vkEnumerateDeviceExtensionProperties(physicalDevice.getHandle(), nullptr, &extensionCount, nullptr);
    supportedExtensions.resize(extensionCount);
    vkEnumerateDeviceExtensionProperties(physicalDevice.getHandle(), nullptr, &extensionCount,
        supportedExtensions.data());

    VkDeviceCreateInfo deviceCreateInfo = {};
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
    
    deviceCreateInfo.pNext = &physicalDeviceFeatures2;
    physicalDeviceFeatures2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
    void** nextFeatureChain = &physicalDeviceFeatures2.pNext;

    physicalDeviceProperties2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2_KHR;
    void** nextPropertyChain = &physicalDeviceProperties2.pNext;

    // Device extensions
    deviceExtensions.push_back(ExtensionFactory::createDeviceExtension(ExtensionName::SwapchainExtension));
    deviceExtensions.push_back(ExtensionFactory::createDeviceExtension(ExtensionName::DebugMarkerExtension));
    deviceExtensions.push_back(ExtensionFactory::createDeviceExtension(ExtensionName::AccelerationStructureExtension));
    deviceExtensions.push_back(ExtensionFactory::createDeviceExtension(ExtensionName::RayTracingPipelineExtension));
    deviceExtensions.push_back(ExtensionFactory::createDeviceExtension(ExtensionName::DeviceAddressExtension));
    deviceExtensions.push_back(ExtensionFactory::createDeviceExtension(ExtensionName::DeferredHostOperationsExtension));
    deviceExtensions.push_back(ExtensionFactory::createDeviceExtension(ExtensionName::RayQuery));
    deviceExtensions.push_back(ExtensionFactory::createDeviceExtension(ExtensionName::DescriptorIndexing));
    deviceExtensions.push_back(ExtensionFactory::createDeviceExtension(ExtensionName::Spirv_1_4));

    for (auto& deviceExtension : deviceExtensions)
    {
        deviceExtension->check(supportedExtensions);
        deviceExtension->add(requestedExtensions);
        deviceExtension->feature(nextFeatureChain);
        deviceExtension->property(devicePropertyMap, nextPropertyChain);
    }

    deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(requestedExtensions.size());
    deviceCreateInfo.ppEnabledExtensionNames = requestedExtensions.data();

    device.init(physicalDevice.getHandle(), deviceCreateInfo);

    for (auto& deviceExtension : deviceExtensions)
    {
        deviceExtension->fetch(device.getHandle());
    }

    physicalDevice.getProperties2(&physicalDeviceProperties2);

    queueFamilyIndex = graphicsQueueIndex;
    commandBufferManager->init(device.getHandle(), graphicsQueueIndex);
    queue->init(device.getHandle(), graphicsQueueIndex);

    LOGD("Done to create logical device");

    VkPipelineCacheCreateInfo pipelineCacheCreateInfo = {};
    pipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
    VKCALL(pipelineCache.init(device.getHandle(), pipelineCacheCreateInfo));

    return true;
}

VkDeviceQueueCreateInfo Context::getQueueCreateInfo(VkQueueFlags queueFlags, uint32_t* queueIndex, float queuePriority)
{
    *queueIndex = getQueueFamilyIndex(VK_QUEUE_GRAPHICS_BIT);
    VkDeviceQueueCreateInfo queueCreateInfo = {};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = *queueIndex;
    queueCreateInfo.queueCount = 1;
    queueCreateInfo.pQueuePriorities = &queuePriority;
    
    return queueCreateInfo;
}

uint32_t Context::getQueueFamilyIndex(VkQueueFlagBits queueFlags) const
{
    ASSERT(surface->validSurface());
    ASSERT(physicalDevice.valid());

    std::vector<VkQueueFamilyProperties> queueFamilyProperties;

    uint32_t queueFamilyCount;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice.getHandle(), &queueFamilyCount, nullptr);
    assert(queueFamilyCount > 0);
    queueFamilyProperties.resize(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice.getHandle(), &queueFamilyCount,
                                             queueFamilyProperties.data());

    std::vector<VkBool32> supportsPresent(queueFamilyCount);
    for (uint32_t i = 0; i < queueFamilyCount; i++)
    {
        vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice.getHandle(), i, surface->getSurface(), &supportsPresent[i]);
    }
    
    // Dedicated queue for compute
    // Try to find a queue family index that supports compute but not graphics
    if (queueFlags & VK_QUEUE_COMPUTE_BIT)
    {
        for (uint32_t i = 0; i < static_cast<uint32_t>(queueFamilyProperties.size()); i++)
        {
            if ((queueFamilyProperties[i].queueFlags & queueFlags) &&
                ((queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0))
            {
                return i;
            }
        }
    }

    // Dedicated queue for transfer
    // Try to find a queue family index that supports transfer but not graphics
    // and compute
    if (queueFlags & VK_QUEUE_TRANSFER_BIT)
    {
        for (uint32_t i = 0; i < static_cast<uint32_t>(queueFamilyProperties.size()); i++)
        {
            if ((queueFamilyProperties[i].queueFlags & queueFlags) &&
                ((queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0) &&
                ((queueFamilyProperties[i].queueFlags & VK_QUEUE_COMPUTE_BIT) == 0))
            {
                return i;
            }
        }
    }

    // For other queue types or if no separate compute queue is present, return
    // the first one to support the requested flags
    for (uint32_t i = 0; i < static_cast<uint32_t>(queueFamilyProperties.size()); i++)
    {
        if (queueFamilyProperties[i].queueFlags & queueFlags)
        {
            return i;
        }
    }

    throw std::runtime_error("Could not find a matching queue family index");
}

uint32_t Context::getNextImageIndex()
{
    surface->acquireNextImage(device.getHandle(), commandBufferManager, queue);
    return surface->getCurrentImageIdex();
}

bool Context::present()
{
    VKCALL(surface->present(device.getHandle(), commandBufferManager, queue));
    queue->waitIdle();
    return true;
}

bool Context::submit()
{
    commandBufferManager->submitActiveCommandBuffer(device.getHandle(), queue);
    queue->waitIdle();
    return true;
}

void Context::wait()
{
    queue->waitIdle();
    commandBufferManager->resetCommandBuffers(device.getHandle(), true);
}

void Context::draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance)
{
    getActiveCommandBuffer()->draw(vertexCount, instanceCount, firstVertex, firstInstance);
}

void Context::drawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, uint32_t vertexOffset, uint32_t firstInstance)
{
    getActiveCommandBuffer()->drawIndexed(indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
}

void Context::dispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ)
{
    getActiveCommandBuffer()->dispatch(groupCountX, groupCountY, groupCountZ);
}

void Context::dispatchIndirect(rhi::StorageBuffer* rhiBuffer)
{
    StorageBuffer* buffer = reinterpret_cast<StorageBuffer*>(rhiBuffer);
    getActiveCommandBuffer()->dispatchIndirect(buffer->getHandle(), 0);
}

CommandBuffer* Context::getActiveCommandBuffer()
{
    ASSERT(commandBufferManager);
    return commandBufferManager->getActiveCommandBuffer(device.getHandle());
}

CommandBuffer* Context::getUploadCommandBuffer()
{
    ASSERT(commandBufferManager);
    return commandBufferManager->getUploadCommandBuffer(device.getHandle());
}

void Context::submitUploadCommandBuffer()
{
    ASSERT(commandBufferManager);
    commandBufferManager->submitUploadCommandBuffer(device.getHandle(), queue);
}

VkDevice Context::getDevice() { return device.getHandle(); }

VkPhysicalDevice Context::getPhysicalDevice() { return physicalDevice.getHandle(); }

Surface* Context::getSurface() { return surface; }

CommandBufferManager* Context::getCommandBufferManager() { return commandBufferManager; }

DescriptorPool* Context::getDescriptorPool() { return descriptorPool; }
}