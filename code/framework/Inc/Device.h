#ifndef __DEVICE_H__
#define __DEVICE_H__

#include <vulkan/vulkan.h>
#include <optional>
#include <vector>

#include "PhysicalDevice.h"

namespace render {

class Device {
public:
    struct ImageMemoryBarrierInfo {
        VkImageLayout oldLayout;
        VkImageLayout newLayout;
        VkPipelineStageFlags srcStage;
        VkAccessFlags srcAccessMask;
        VkPipelineStageFlags dstStage;
        VkAccessFlags dstAccessMask;
    };

public:
    Device() {}
    ~Device() {}

    /*
     * @param physicalDevice Must be a non-null pointer.
     */
    void Init(PhysicalDevice* physicalDevice);

    void CleanUp();

    bool IsValid() { return mDevice != VK_NULL_HANDLE; }

    VkDevice Get() { return mDevice; }

    PhysicalDevice* GetPhysicalDevice() { return mPhysicalDevice; }

    VkQueue GetGraphicsQueue() { return mGraphicsQueue; }

    VkQueue GetPresentQueue() { return mPresentQueue; }

    void CreateImage(VkImageCreateInfo* pImageInfo, VkMemoryPropertyFlags properties,
        VkImage& image, VkDeviceMemory& imageMemory);

    VkCommandBuffer CreateCommandBuffer(VkCommandBufferLevel level);

    void FreeCommandBuffer(VkCommandBuffer commandBuffer);

    void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
        VkBuffer& buffer, VkDeviceMemory& bufferMemory);

    VkCommandBuffer BeginSingleTimeCommands();

    void EndSingleTimeCommands(VkCommandBuffer commandBuffer);

    void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

    void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

    void TransitionImageLayout(VkImage image, VkImageAspectFlags aspectMask,
        const ImageMemoryBarrierInfo& barrierInfo, uint32_t mipLevels = 1);

    void AddCmdPipelineBarrier(VkCommandBuffer cmdBuffer, VkImage image, VkImageAspectFlags aspectMask,
        const ImageMemoryBarrierInfo& barrierInfo, uint32_t mipLevels = 1);

private:
    void CreateLogicalDevice();
    void CreateCommandPool();

private:
    // external objects
    PhysicalDevice* mPhysicalDevice = nullptr;

    // device
    VkDevice mDevice = VK_NULL_HANDLE;	                // 逻辑设备
    VkQueue mGraphicsQueue = VK_NULL_HANDLE;	 // 图形队列
    VkQueue mPresentQueue = VK_NULL_HANDLE;	     // 显示队列
    VkCommandPool mCommandPoolOfGraphics = VK_NULL_HANDLE; // 命令池

    // info
    PhysicalDevice::QueueFamilyIndices mQueueFamilyIndices = {};
};
}


#endif // !__DEVICE_H__


