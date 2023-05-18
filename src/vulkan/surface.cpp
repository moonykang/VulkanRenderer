#include <vector>
#include "platform/window.h"
#include "vulkan/surface.h"
#include "vulkan/commandBufferManager.h"
#include "vulkan/queue.h"

namespace vk
{
Surface::Surface()
    : surface(VK_NULL_HANDLE)
    , surfaceSize(EMPTY_STRUCT)
    , swapchain(VK_NULL_HANDLE)
	, currentImageIndex(0)
	, imageCount(0)
{

}

Surface::~Surface()
{

}

void Surface::destroySurface(VkInstance instance)
{
    if (surface != VK_NULL_HANDLE)
    {
        vkDestroySurfaceKHR(instance, surface, nullptr);
        surface = VK_NULL_HANDLE;
    }
}

void Surface::selectSurfaceFormat(VkPhysicalDevice physicalDevice)
{

	// Get list of supported surface formats
	uint32_t formatCount;
	VKCALL(vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, NULL));
	assert(formatCount > 0);

	std::vector<VkSurfaceFormatKHR> surfaceFormats(formatCount);
	VKCALL(vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, surfaceFormats.data()));

	// If the surface format list only includes one entry with VK_FORMAT_UNDEFINED,
	// there is no preferred format, so we assume VK_FORMAT_B8G8R8A8_UNORM
	if ((formatCount == 1) && (surfaceFormats[0].format == VK_FORMAT_UNDEFINED))
	{
		surfaceFormat.format = VK_FORMAT_B8G8R8A8_UNORM;
		surfaceFormat.colorSpace = surfaceFormats[0].colorSpace;
	}
	else
	{
		// iterate over the list of available surface format and
		// check for the presence of VK_FORMAT_B8G8R8A8_UNORM
		bool found_rgba8 = false;
		for (auto&& surfaceformat : surfaceFormats)
		{
			if (surfaceformat.format == VK_FORMAT_B8G8R8A8_UNORM)
			{
				surfaceFormat = surfaceformat;
				found_rgba8 = true;
				break;
			}
		}

		// in case VK_FORMAT_B8G8R8A8_UNORM is not available
		// select the first available color format
		if (!found_rgba8)
		{
			surfaceFormat = surfaceFormats[0];
		}
	}
}

bool Surface::initSwapchain(VkPhysicalDevice physicalDevice, VkDevice device)
{
	// Store the current swap chain handle so we can use it later on to ease up recreation
	VkSwapchainKHR oldSwapchain = swapchain;

	// Get physical device surface properties and formats
	VKCALL(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &surfaceCapabilities));

	// Get available present modes
	uint32_t presentModeCount;
	VKCALL(vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, NULL));
	assert(presentModeCount > 0);

	std::vector<VkPresentModeKHR> presentModes(presentModeCount);
	VKCALL(vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, presentModes.data()));

	uint32_t desiredHeight = 720;
	double desireHeightRatio = (double) desiredHeight / surfaceSize.height;
	uint32_t desiredWidth = surfaceSize.width * desireHeightRatio;

	VkExtent2D swapchainExtent = { desiredWidth, desiredHeight};
	surfaceSize = swapchainExtent;

	VkPresentModeKHR swapchainPresentMode = VK_PRESENT_MODE_FIFO_KHR;

	uint32_t desiredNumberOfSwapchainImages = surfaceCapabilities.minImageCount + 1;
	if ((surfaceCapabilities.maxImageCount > 0) && (desiredNumberOfSwapchainImages > surfaceCapabilities.maxImageCount))
	{
		desiredNumberOfSwapchainImages = surfaceCapabilities.maxImageCount;
	}

	// Find the transformation of the surface
	VkSurfaceTransformFlagsKHR preTransform;
	if (surfaceCapabilities.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
	{
		// We prefer a non-rotated transform
		preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	}
	else
	{
		preTransform = surfaceCapabilities.currentTransform;
	}


	// Find a supported composite alpha format (not all devices support alpha opaque)
	VkCompositeAlphaFlagBitsKHR compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	// Simply select the first composite alpha format available
	std::vector<VkCompositeAlphaFlagBitsKHR> compositeAlphaFlags = {
		VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
		VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
		VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR,
		VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR,
	};
	for (auto& compositeAlphaFlag : compositeAlphaFlags) {
		if (surfaceCapabilities.supportedCompositeAlpha & compositeAlphaFlag) {
			compositeAlpha = compositeAlphaFlag;
			break;
		};
	}

	selectSurfaceFormat(physicalDevice);

	VkSwapchainCreateInfoKHR swapchainCreateInfo = {};
	swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapchainCreateInfo.surface = surface;
	swapchainCreateInfo.minImageCount = desiredNumberOfSwapchainImages;
	swapchainCreateInfo.imageFormat = surfaceFormat.format;
	swapchainCreateInfo.imageColorSpace = surfaceFormat.colorSpace;
	swapchainCreateInfo.imageExtent = swapchainExtent;
	swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	swapchainCreateInfo.preTransform = (VkSurfaceTransformFlagBitsKHR)preTransform;
	swapchainCreateInfo.imageArrayLayers = 1;
	swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	swapchainCreateInfo.queueFamilyIndexCount = 0;
	swapchainCreateInfo.presentMode = swapchainPresentMode;
	// Setting oldSwapChain to the saved handle of the previous swapchain aids in resource reuse and makes sure that we can still present already acquired images
	swapchainCreateInfo.oldSwapchain = oldSwapchain;
	// Setting clipped to VK_TRUE allows the implementation to discard rendering outside of the surface area
	swapchainCreateInfo.clipped = VK_TRUE;
	swapchainCreateInfo.compositeAlpha = compositeAlpha;

	// Enable transfer source on swap chain images if supported
	if (surfaceCapabilities.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_SRC_BIT) {
		swapchainCreateInfo.imageUsage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
	}

	// Enable transfer destination on swap chain images if supported
	if (surfaceCapabilities.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_DST_BIT) {
		swapchainCreateInfo.imageUsage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	}

	VKCALL(vkCreateSwapchainKHR(device, &swapchainCreateInfo, nullptr, &swapchain));

	// If an existing swap chain is re-created, destroy the old swap chain
	// This also cleans up all the presentable images
	if (oldSwapchain != VK_NULL_HANDLE)
	{
		// TODO:
		for (auto& swapchainImage : swapchainImages)
		{
			swapchainImage->destroy(device);
		}
		swapchainImages.clear();
		vkDestroySwapchainKHR(device, oldSwapchain, nullptr);
	}

	std::vector<VkImage> images;
	VKCALL(vkGetSwapchainImagesKHR(device, swapchain, &imageCount, NULL));

	// Get the swap chain images
	images.resize(imageCount);
	VKCALL(vkGetSwapchainImagesKHR(device, swapchain, &imageCount, images.data()));

	swapchainImages.resize(imageCount);

	for (uint32_t i = 0; i < imageCount; i++)
	{
		auto& swapchainImage = swapchainImages[i];
		swapchainImage = new vk::Image();
		swapchainImage->setHandle(images[i], surfaceFormat.format, swapchainExtent);
		swapchainImage->createImageView(device, surfaceFormat.format, getImageAspectMask(surfaceFormat.format));
	}

	acquireSemaphores.resize(imageCount);
	presentSemaphores.resize(imageCount);

	for (auto& semaphore : acquireSemaphores)
	{
		semaphore.init(device);
	}

	for (auto& semaphore : presentSemaphores)
	{
		semaphore.init(device);
	}

    return true;
}

void Surface::destroySwapchain(VkDevice device)
{
	if (swapchain != VK_NULL_HANDLE)
	{
		for (auto& semaphore : acquireSemaphores)
		{
			semaphore.destroy(device);
		}
		acquireSemaphores.clear();

		for (auto& semaphore : presentSemaphores)
		{
			semaphore.destroy(device);
		}
		presentSemaphores.clear();

		for (auto& swapchainImage : swapchainImages)
		{
			swapchainImage->destroyImageView(device);
		}
		swapchainImages.clear();
		vkDestroySwapchainKHR(device, swapchain, nullptr);
		swapchain = VK_NULL_HANDLE;
	}
}

bool Surface::validSurface() const { return surface != VK_NULL_HANDLE; }

VkSurfaceKHR Surface::getSurface() { return surface; }

VkExtent2D Surface::getSurfaceSize() { return surfaceSize; }

std::vector<vk::Image*>& Surface::getSwapchainImages() { return swapchainImages; }

VkResult Surface::acquireNextImage(VkDevice device, CommandBufferManager* commandBufferManager, Queue* queue)
{
	VkSemaphore semaphore = acquireSemaphores[currentImageIndex].getHandle();
	VkResult result = vkAcquireNextImageKHR(device, swapchain, UINT64_MAX, semaphore, VK_NULL_HANDLE, &currentImageIndex);
	return result;
}

uint32_t Surface::getCurrentImageIdex() { return currentImageIndex; }

VkResult Surface::present(VkDevice device, CommandBufferManager* commandBufferManager, Queue* queue)
{
	auto image = swapchainImages[currentImageIndex];
	CommandBuffer* commandBuffer = commandBufferManager->getActiveCommandBuffer(device);
	commandBuffer->addTransition(image->updateImageLayoutAndBarrier(ImageLayout::Present));
	std::vector<VkSemaphore> waitSemaphores = { acquireSemaphores[currentImageIndex].getHandle() };
	std::vector<VkSemaphore> signalSemaphores = { presentSemaphores[currentImageIndex].getHandle() };
	commandBufferManager->submitActiveCommandBuffer(device, queue, &waitSemaphores, &signalSemaphores);

	VkResult vkResult = queue->present(swapchain, currentImageIndex, &signalSemaphores);
	setNextImageIndex();
	return vkResult;
}

void Surface::updateSurfaceSize(int width, int height)
{
	surfaceSize.width = width;
	surfaceSize.height = height;
}

VkSurfaceFormatKHR Surface::getSurfaceFormat() { return surfaceFormat; }

}