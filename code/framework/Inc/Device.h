#ifndef __DEVICE_H__
#define __DEVICE_H__

#include <vulkan/vulkan.h>
#include <optional>
#include <vector>

#include "PhysicalDevice.h"

namespace framework {

struct ImageMemoryBarrierInfo {
    VkImageLayout oldLayout;
    VkImageLayout newLayout;
    VkPipelineStageFlags srcStage;
    VkAccessFlags srcAccessMask;
    VkPipelineStageFlags dstStage;
    VkAccessFlags dstAccessMask;
};

class Device {
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

    VkCommandBuffer CreateCommandBuffer(VkCommandBufferLevel level);

    void FreeCommandBuffer(VkCommandBuffer commandBuffer);

    VkCommandBuffer BeginSingleTimeCommands();

    void EndSingleTimeCommands(VkCommandBuffer commandBuffer);

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
}   // namespace framework


#endif // !__DEVICE_H__


