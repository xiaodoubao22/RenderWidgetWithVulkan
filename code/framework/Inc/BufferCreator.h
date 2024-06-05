#ifndef __BUFFER_CREATOR__
#define __BUFFER_CREATOR__

#include <vulkan/vulkan.h>
#include "Device.h"
#include "Utils.h"
#include "SceneDemoDefs.h"

#include "VmaUsage.h"

namespace framework {

class BufferCreator {
public:
    BufferCreator() {};
    ~BufferCreator() {};

    static BufferCreator& GetInstance();

    void Init(Device* device);

    void CleanUp();

    VmaAllocator GetAllocator() {
        return mAllocator;
    }

    void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
        VkBuffer& buffer, VkDeviceMemory& bufferMemory);

    void CreateImage(VkImageCreateInfo* pImageInfo, VkMemoryPropertyFlags properties,
        VkImage& image, VkDeviceMemory& imageMemory);

    void CreateImages(std::vector<VkImageCreateInfo>& imageInfos, VkMemoryPropertyFlags properties,
        std::vector<VkImage>& images, VkDeviceMemory& imageMemory);

    void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

    void CopyBufferToImage(VkBuffer buffer, VkImage image, VkExtent3D imageExtent);

    void TransitionImageLayout(VkImage image, VkImageAspectFlags aspectMask,
        const ImageMemoryBarrierInfo& barrierInfo, uint32_t mipLevels = 1);

    void CreateBufferFromSrcData(VkBufferUsageFlags usage, void* srcData, VkDeviceSize dataSize,
        VkBuffer& buffer, VkDeviceMemory& bufferMemory);

    void CreateTextureFromSrcData(VkImageCreateInfo imageInfo, void* srcImage, VkDeviceSize imageSize,
        VkImage& image, VkDeviceMemory& imageMemory);

    void CreateTextureFromSrcData(VkImageCreateInfo imageInfo, void* srcImage, VkDeviceSize imageSize,
        VkImage& image, VmaAllocation& imageAllocation);

    void CreateTexturesFromSrcData(std::vector<VkImageCreateInfo>& imageInfos, std::vector<StbImageBuffer>& imageDataList,
        std::vector<VkImage>& images, VkDeviceMemory& imageMemory);

    void CreateTexturesFromSrcData(std::vector<VkImageCreateInfo>& imageInfos, std::vector<StbImageBuffer>& imageDataList,
        std::vector<VkImage>& images, std::vector<VmaAllocation>& imageAllocation);

    void CreateMappedBuffers(std::vector<VkBufferCreateInfo>& bufferInfos, std::vector<VkBuffer>& buffers,
        std::vector<void*>& mappedAddress, VkDeviceMemory& bufferMemory);
private:
    Device* mDevice = nullptr;
    VmaAllocator mAllocator = VK_NULL_HANDLE;

    bool mInited = false;
};

} // namespace framework

#endif // !__BUFFER_CREATOR__
