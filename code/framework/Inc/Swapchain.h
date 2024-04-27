#ifndef __SWAPCHAIN_H__
#define __SWAPCHAIN_H__

#include <vulkan/vulkan.h>
#include <vector>

#include "PhysicalDevice.h"
#include "Device.h"

namespace window {
class WindowTemplate;
}

namespace framework {
struct SwapChainSupportdetails {
    bool isValid = false;
    VkSurfaceCapabilitiesKHR capabilities = {};
    std::vector<VkSurfaceFormatKHR> formats = {};
    std::vector<VkPresentModeKHR> presentModes = {};
};

class Swapchain {
public:
    Swapchain();
    ~Swapchain();

    bool Init(PhysicalDevice* physicalDevice, Device* device, VkExtent2D windowExtent, VkSurfaceKHR surface);
    bool CleanUp();
    bool Recreate(VkExtent2D windowExtent);

    VkFormat GetFormat() { return mSwapchainImageFormat; }
    std::vector<VkImageView> GetImageViews() { return mSwapchainImageViews; }
    VkExtent2D GetExtent() { return mSwapchainExtent; }

    bool AcquireImage(VkSemaphore imageAvailiableSemaphore, uint32_t& imageIndex);
    bool QueuePresent(uint32_t imageIndex, const std::vector<VkSemaphore>& waitSemaphores);

private:
    void CreateSwapChain(VkExtent2D windowExtent, VkSwapchainKHR oldSwapchain);
    void CreateImageViews();
    void DestroyImageViews();

    VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
    VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availiablePresentModes);
    VkExtent2D ChooseSwapExtent(VkExtent2D windowExtent, const VkSurfaceCapabilitiesKHR& capabilities);
    VkSharingMode ChooseSharingMode(const PhysicalDevice::QueueFamilyIndices& indices, std::vector<uint32_t>& indicesList);

private:
    bool mIsInitialized = false;

    // external objects
    //GraphicsDevice* mGraphicsDevice = nullptr;
    PhysicalDevice* mPhysicalDevice = nullptr;
    Device* mDevice = nullptr;
    VkSurfaceKHR mSurface = VK_NULL_HANDLE;

    // infos
    VkFormat mSwapchainImageFormat = {};
    VkPresentModeKHR mPresentMode = {};
    VkExtent2D mSwapchainExtent = {};
    uint32_t mImageCount = 0;

    // swapchain
    VkSwapchainKHR mSwapChain = VK_NULL_HANDLE;

    // images
    std::vector<VkImage> mSwapchainImages = {};
    std::vector<VkImageView> mSwapchainImageViews = {};
};
}   // namespace framework


#endif // !__SWAPCHAIN_H__

