#pragma once

#include <vector>
#include "vulkan/image.h"
#include "vulkan/vk_wrapper.h"

namespace platform
{
class Window;
}

namespace vk
{
class Image;
class CommandBufferManager;
class Queue;

class Surface
{
public:
    Surface();

    ~Surface();

    bool initSurface(VkInstance instance, platform::Window* window);

    void destroySurface(VkInstance instance);

    bool initSwapchain(VkPhysicalDevice physicalDevice, VkDevice device);

    void destroySwapchain(VkDevice device);
public:
    bool validSurface() const;

    VkSurfaceKHR getSurface();

    VkExtent2D getSurfaceSize();

    std::vector<vk::Image*>& getSwapchainImages();

    VkResult acquireNextImage(VkDevice device, CommandBufferManager* commandBufferManager, Queue* queue);

    uint32_t getCurrentImageIdex();

    void setNextImageIndex() { currentImageIndex = (currentImageIndex + 1) % imageCount; }

    VkResult present(VkDevice device, CommandBufferManager* commandBufferManager, Queue* queue);
private:
    void updateSurfaceSize(int width, int height);

    void selectSurfaceFormat(VkPhysicalDevice physicalDevice);

public:
    VkSurfaceFormatKHR getSurfaceFormat();

    VkFormat getFormat() { return surfaceFormat.format; }

private:
    VkSurfaceKHR surface;
    VkSwapchainKHR swapchain;
    VkExtent2D surfaceSize;
    VkSurfaceFormatKHR surfaceFormat;

    std::vector<vk::Image*> swapchainImages;
    VkSurfaceCapabilitiesKHR surfaceCapabilities;

    uint32_t currentImageIndex;
    uint32_t imageCount;
    std::vector<handle::Semaphore> acquireSemaphores;
    std::vector<handle::Semaphore> presentSemaphores;
};
}