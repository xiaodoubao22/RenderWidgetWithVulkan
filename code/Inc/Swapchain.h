#ifndef __SWAPCHAIN_H__
#define __SWAPCHAIN_H__

#include <vulkan/vulkan.h>
#include <vector>

#include "GraphicsDevice.h"

namespace render {
    class Swapchain {
    public:
        Swapchain();
        ~Swapchain();

        void Init(GraphicsDevice* graphicsDevice, VkExtent2D windowExtent, VkSurfaceKHR surface);
        void CleanUp();

        VkSwapchainKHR GetSwapchain() { return mSwapChain; }
        VkFormat GetFormat() { return mSwapChainImageFormat; }
        std::vector<VkImageView> GetImageViews() { return mSwapChainImageViews; }
        VkExtent2D GetExtent() { return mWindowExtent; }

        uint32_t AcquireImage(VkSemaphore imageAvailiableSemaphore);
        VkResult QueuePresent(uint32_t imageIndex, const std::vector<VkSemaphore>& waitSemaphores);

    private:
        void CreateSwapChain();
        void CreateImageViews();
        void DestroyImageViews();

        VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
        VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availiablePresentModes);
        VkExtent2D ChooseSwapExtent(VkExtent2D windowExtent, const VkSurfaceCapabilitiesKHR& capabilities);
        VkSharingMode ChooseSharingMode(const QueueFamilyIndices& indices, std::vector<uint32_t>& indicesList);

    private:
        // external objects
        GraphicsDevice* mGraphicsDevice;
        VkExtent2D mWindowExtent = {};
        VkSurfaceKHR mSurface = VK_NULL_HANDLE;

        // swapchain
        VkSwapchainKHR mSwapChain = VK_NULL_HANDLE;
        VkFormat mSwapChainImageFormat = {};
        VkExtent2D mSwapChainExtent = {};

        // images
        std::vector<VkImage> mSwapChainImages = {};
        std::vector<VkImageView> mSwapChainImageViews = {};
    };
}


#endif // !__SWAPCHAIN_H__

