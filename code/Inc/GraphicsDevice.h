#ifndef __GRAPHICS_DEVICE_H__
#define __GRAPHICS_DEVICE_H__

#include <vulkan/vulkan.h>
#include <optional>
#include <vector>

namespace render {
    struct QueueFamilyIndices {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentFamily;

        bool IsComplete() {
            return graphicsFamily.has_value() && presentFamily.has_value();
        }
    };

    struct SwapChainSupportdetails {
        bool isValid = false;
        VkSurfaceCapabilitiesKHR capabilities = {};
        std::vector<VkSurfaceFormatKHR> formats = {};
        std::vector<VkPresentModeKHR> presentModes = {};
    };

    struct ImageMemoryBarrierInfo {
        VkImageLayout oldLayout;
        VkImageLayout newLayout;
        VkPipelineStageFlags srcStage;
        VkAccessFlags srcAccessMask;
        VkPipelineStageFlags dstStage;
        VkAccessFlags dstAccessMask;
    };

    class GraphicsDevice {
    public:
        GraphicsDevice();
        ~GraphicsDevice();

        void Init(VkInstance instance, VkSurfaceKHR supportedSurface);
        void CleanUp();

        VkDevice GetDevice() { return mDevice; }
        VkQueue GetGraphicsQueue() { return mGraphicsQueue; }
        VkQueue GetPresentQueue() { return mPresentQueue; }
        QueueFamilyIndices GetQueueFamilies() { return FindQueueFamilies(mPhysicalDevice); }
        VkSurfaceKHR GetSupportedSurface() { return mSupportedSurface; }

        SwapChainSupportdetails QuerySwapChainSupport();
        VkFormat FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
        uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
        void CreateImage(VkImageCreateInfo* pImageInfo, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
        VkCommandBuffer CreateCommandBuffer(VkCommandBufferLevel level);
        void FreeCommandBuffer(VkCommandBuffer commandBuffer);
        void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
            VkBuffer& buffer, VkDeviceMemory& bufferMemory);
        VkCommandBuffer BeginSingleTimeCommands();
        void EndSingleTimeCommands(VkCommandBuffer commandBuffer);
        void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
        void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
        void TransitionImageLayout(VkImage image, VkImageAspectFlags aspectMask, const ImageMemoryBarrierInfo& barrierInfo, uint32_t mipLevels = 1);

    private:
        void PickPhysicalDevices();
        void CreateLogicalDevice();
        void CreateCommandPool();

        // tool functions
        int RateDeviceSuitability(VkPhysicalDevice device);
        QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device);

    private:
        bool mIsInitialized = false;

        VkInstance mInstance = VK_NULL_HANDLE;
        VkSurfaceKHR mSupportedSurface = VK_NULL_HANDLE;

        VkPhysicalDevice mPhysicalDevice = VK_NULL_HANDLE;	// 物理设备
        VkDevice mDevice = VK_NULL_HANDLE;	                // 逻辑设备
        VkQueue mGraphicsQueue = VK_NULL_HANDLE;	 // 图形队列
        VkQueue mPresentQueue = VK_NULL_HANDLE;	     // 显示队列
        VkCommandPool mCommandPoolOfGraphics = VK_NULL_HANDLE; // 命令池

        QueueFamilyIndices mQueueFamilyIndices = {};
    };
}


#endif // !__GRAPHICS_DEVICE_H__

