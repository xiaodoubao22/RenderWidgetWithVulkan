#ifndef __BUFFER_CREATOR__
#define __BUFFER_CREATOR__

#include <vulkan/vulkan.h>
#include "Device.h"
#include "Utils.h"

namespace framework {

class BufferCreator {
public:
    BufferCreator() {};
    ~BufferCreator() {};

    static BufferCreator& GetInstance() {
        static BufferCreator instance;
        return instance;
    }

    void SetDevice(Device* device) {
        mDevice = device;
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

    void CreateTexturesFromSrcData(std::vector<VkImageCreateInfo>& imageInfos, std::vector<StbImageBuffer>& imageDataList,
        std::vector<VkImage>& images, VkDeviceMemory& imageMemory);

    void CreateMappedBuffers(std::vector<VkBufferCreateInfo>& bufferInfos, std::vector<VkBuffer>& buffers,
        std::vector<void*>& mappedAddress, VkDeviceMemory& bufferMemory);
private:
    Device* mDevice = nullptr;
};

} // namespace framework

#endif // !__BUFFER_CREATOR__
