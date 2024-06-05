#include "BufferCreator.h"
#include <stdexcept>

#include "VulkanInitializers.h"
#include "Log.h"
#undef LOG_TAG
#define LOG_TAG "BufferCreator"

namespace framework {
BufferCreator& BufferCreator::GetInstance() {
    static BufferCreator instance;
    return instance;
}

void BufferCreator::Init(Device* device)
{
    if (mInited) {
        LOGE("repeated init");
        return;
    }
    if (device == nullptr) {
        return;
    }

    VmaAllocatorCreateInfo allocatorCreateInfo = {};
    allocatorCreateInfo.vulkanApiVersion = GetConfig().instance.apiVersion;
    allocatorCreateInfo.physicalDevice = device->GetPhysicalDevice()->Get();
    allocatorCreateInfo.device = device->Get();
    allocatorCreateInfo.instance = device->GetPhysicalDevice()->GetInstance();
    vmaCreateAllocator(&allocatorCreateInfo, &mAllocator);

    mDevice = device;
    mInited = true;
}

void BufferCreator::CleanUp()
{
    if (!mInited) {
        return;
    }
    vmaDestroyAllocator(mAllocator);
    mAllocator = VK_NULL_HANDLE;

    mDevice = nullptr;
    mInited = false;
}
void BufferCreator::CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
    VkBuffer& buffer, VkDeviceMemory& bufferMemory)
{
    if (mDevice == nullptr || mDevice->GetPhysicalDevice() == nullptr) {
        throw std::runtime_error("mPhysicalDevice/mDevice is null!");
    }

    // 创建缓冲区
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size; // 缓冲区大小
    bufferInfo.usage = usage;     // 缓冲区用途
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;         // 指定独占模式，因为只有一个队列（图形队列）需要用它
    bufferInfo.flags = 0;
    if (vkCreateBuffer(mDevice->Get(), &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to create vertex buffer");
    }

    // 该buffer对显存的要求
    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(mDevice->Get(), buffer, &memRequirements);

    // 申请显存
    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;        // 显存大小
    allocInfo.memoryTypeIndex = mDevice->GetPhysicalDevice()->FindMemoryType(memRequirements.memoryTypeBits, properties);
    if (vkAllocateMemory(mDevice->Get(), &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate vertex buffer memory!");
    }

    // 显存绑定到缓冲区
    vkBindBufferMemory(mDevice->Get(), buffer, bufferMemory, 0);
}

void BufferCreator::CreateImage(VkImageCreateInfo* pImageInfo, VkMemoryPropertyFlags properties,
    VkImage& image, VkDeviceMemory& imageMemory)
{
    if (mDevice == nullptr || mDevice->GetPhysicalDevice() == nullptr) {
        throw std::runtime_error("mPhysicalDevice/mDevice is null!");
    }

    if (vkCreateImage(mDevice->Get(), pImageInfo, nullptr, &image) != VK_SUCCESS) {    // 创建VkImage
        throw std::runtime_error("failed to create image!");
    }

    // 为VkImage分配显存
    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(mDevice->Get(), image, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = mDevice->GetPhysicalDevice()->FindMemoryType(memRequirements.memoryTypeBits, properties);
    if (vkAllocateMemory(mDevice->Get(), &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate image memory");
    }

    vkBindImageMemory(mDevice->Get(), image, imageMemory, 0);
}

void BufferCreator::CreateImages(std::vector<VkImageCreateInfo>& imageInfos, VkMemoryPropertyFlags properties,
    std::vector<VkImage>& images, VkDeviceMemory& imageMemory)
{
    if (mDevice == nullptr || mDevice->GetPhysicalDevice() == nullptr) {
        throw std::runtime_error("mPhysicalDevice/mDevice is null!");
        return;
    }
    if (imageInfos.size() <= 0) {
        throw std::runtime_error("pImageInfos.size == 0");
        return;
    }

    // create image
    images = std::vector<VkImage>(imageInfos.size(), VK_NULL_HANDLE);
    for (int i = 0; i < imageInfos.size(); i++) {
        if (vkCreateImage(mDevice->Get(), &imageInfos[i], nullptr, &images[i]) != VK_SUCCESS) {    // 创建VkImage
            throw std::runtime_error("failed to create image!");
        }
    }

    // calculate memory size
    std::vector<VkMemoryRequirements> memRequirements(imageInfos.size());
    for (int i = 0; i < imageInfos.size(); i++) {
        vkGetImageMemoryRequirements(mDevice->Get(), images[i], &memRequirements[i]);
        LOGD("size=%d align=%d", memRequirements[i].size, memRequirements[i].alignment);
    }
    VkDeviceSize totalSize = 0;
    uint32_t memoryType = UINT32_MAX;
    std::vector<VkDeviceSize> offsets(memRequirements.size(), 0);
    for (int i = 0; i < memRequirements.size(); i++) {
        VkDeviceSize alignment = memRequirements[i].alignment;
        offsets[i] = ((totalSize + alignment - 1) / alignment) * alignment;
        totalSize = offsets[i] + memRequirements[i].size;
        memoryType &= memRequirements[i].memoryTypeBits;
        LOGD("totalsize=%d offset=%d", totalSize, offsets[i]);
    }

    // alloc memory
    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = totalSize;
    allocInfo.memoryTypeIndex = mDevice->GetPhysicalDevice()->FindMemoryType(memoryType, properties);
    if (vkAllocateMemory(mDevice->Get(), &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate image memory");
    }

    for (int i = 0; i < images.size(); i++) {
        vkBindImageMemory(mDevice->Get(), images[i], imageMemory, offsets[i]);
        LOGD("offset=%d", offsets[i]);
    }
}

void BufferCreator::CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
{
    if (mDevice == nullptr) {
        throw std::runtime_error("mDevice is null!");
    }
    // 申请并启动一个命令缓冲
    VkCommandBuffer commandBuffer = mDevice->BeginSingleTimeCommands();

    // 写入复制命令
    VkBufferCopy copyRegion{};
    copyRegion.srcOffset = 0;
    copyRegion.dstOffset = 0;
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

    // 提交命令缓冲
    mDevice->EndSingleTimeCommands(commandBuffer);
}

void BufferCreator::CopyBufferToImage(VkBuffer buffer, VkImage image, VkExtent3D imageExtent)
{
    if (mDevice == nullptr) {
        throw std::runtime_error("mDevice is null!");
    }
    VkCommandBuffer commandBuffer = mDevice->BeginSingleTimeCommands();

    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;

    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;

    region.imageOffset = { 0, 0, 0 };
    region.imageExtent = imageExtent;

    vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    mDevice->EndSingleTimeCommands(commandBuffer);
}

void BufferCreator::TransitionImageLayout(VkImage image, VkImageAspectFlags aspectMask,
    const ImageMemoryBarrierInfo& barrierInfo, uint32_t mipLevels)
{
    if (mDevice == nullptr) {
        throw std::runtime_error("mDevice is null!");
    }

    VkCommandBuffer commandBuffer = mDevice->BeginSingleTimeCommands();

    mDevice->AddCmdPipelineBarrier(commandBuffer, image, aspectMask, barrierInfo, mipLevels);

    mDevice->EndSingleTimeCommands(commandBuffer);
}

void BufferCreator::CreateBufferFromSrcData(VkBufferUsageFlags usage, void* srcData, VkDeviceSize dataSize,
    VkBuffer& buffer, VkDeviceMemory& bufferMemory)
{
    if (mDevice == nullptr) {
        throw std::runtime_error("mDevice is null!");
    }
    if (srcData == nullptr || dataSize == 0) {
        throw std::runtime_error("srcData is null!");
    }

    // 创建临时缓冲
    VkBuffer stagingBuffer = VK_NULL_HANDLE;
    VkDeviceMemory stagingBufferMemory = VK_NULL_HANDLE;
    CreateBuffer(dataSize,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        stagingBuffer, stagingBufferMemory);

    // 数据拷贝到临时缓冲
    void* data;
    vkMapMemory(mDevice->Get(), stagingBufferMemory, 0, dataSize, 0, &data);
    memcpy(data, srcData, (size_t)dataSize);
    vkUnmapMemory(mDevice->Get(), stagingBufferMemory);

    // 创建buffer
    CreateBuffer(dataSize,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | usage,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        buffer, bufferMemory);

    // 复制 stagingBuffer -> buffer
    CopyBuffer(stagingBuffer, buffer, dataSize);

    // 清理临时缓冲
    vkDestroyBuffer(mDevice->Get(), stagingBuffer, nullptr);
    vkFreeMemory(mDevice->Get(), stagingBufferMemory, nullptr);
}

void BufferCreator::CreateTextureFromSrcData(VkImageCreateInfo imageInfo, void* srcImage, VkDeviceSize imageSize,
    VkImage& image, VkDeviceMemory& imageMemory)
{
    if (mDevice == nullptr) {
        throw std::runtime_error("mDevice is null!");
    }
    if (srcImage == nullptr || imageSize == 0) {
        throw std::runtime_error("srcImage is null!");
    }

    // 创建临时缓冲
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    CreateBuffer(imageSize,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        stagingBuffer, stagingBufferMemory);

    // 图像数据拷贝到临时缓冲
    void* data;
    vkMapMemory(mDevice->Get(), stagingBufferMemory, 0, imageSize, 0, &data);
    memcpy(data, srcImage, static_cast<size_t>(imageSize));
    vkUnmapMemory(mDevice->Get(), stagingBufferMemory);

    // 创建纹理图像
    imageInfo.usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    CreateImage(&imageInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        image, imageMemory);

    VkCommandBuffer commandBuffer = mDevice->BeginSingleTimeCommands();
    {
        // 转换image格式，undefined -> transferDst
        ImageMemoryBarrierInfo imageBarrierInfo{};
        imageBarrierInfo.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageBarrierInfo.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        imageBarrierInfo.srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        imageBarrierInfo.srcAccessMask = 0;
        imageBarrierInfo.dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        imageBarrierInfo.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        mDevice->AddCmdPipelineBarrier(commandBuffer, image, VK_IMAGE_ASPECT_COLOR_BIT, imageBarrierInfo);

        // 拷贝数据，从buffer到image
        VkBufferImageCopy region{};
        region.bufferOffset = 0;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;
        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = 1;
        region.imageOffset = { 0, 0, 0 };
        region.imageExtent = imageInfo.extent;
        vkCmdCopyBufferToImage(commandBuffer, stagingBuffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

        // 转换image格式，transferDst -> shaderReadOnly
        imageBarrierInfo.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        imageBarrierInfo.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageBarrierInfo.srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        imageBarrierInfo.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        imageBarrierInfo.dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        imageBarrierInfo.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        mDevice->AddCmdPipelineBarrier(commandBuffer, image, VK_IMAGE_ASPECT_COLOR_BIT, imageBarrierInfo);
    }
    mDevice->EndSingleTimeCommands(commandBuffer);

    // 清理临时缓冲
    vkDestroyBuffer(mDevice->Get(), stagingBuffer, nullptr);
    vkFreeMemory(mDevice->Get(), stagingBufferMemory, nullptr);
}

void BufferCreator::CreateTextureFromSrcData(VkImageCreateInfo imageInfo, void* srcImage, VkDeviceSize imageSize,
    VkImage& image, VmaAllocation& imageAllocation)
{
    if (mDevice == nullptr) {
        LOGE("uninitialized");
    }
    if (srcImage == nullptr || imageSize == 0) {
        LOGE("input invalid");
    }

    // 创建临时缓冲
    VkBuffer stagingBuffer = VK_NULL_HANDLE;
    VmaAllocation  stagingBufferAllocation = VK_NULL_HANDLE;

    VkBufferCreateInfo stagingBufferInfo = vulkanInitializers::BufferCreateInfo(
        imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
    VmaAllocationCreateInfo stagingBufferAllocInfo = {};
    stagingBufferAllocInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;
    vmaCreateBuffer(mAllocator, &stagingBufferInfo, &stagingBufferAllocInfo,
        &stagingBuffer, &stagingBufferAllocation, nullptr);

    // 图像数据拷贝到临时缓冲
    void* data;
    vmaMapMemory(mAllocator, stagingBufferAllocation, &data);
    memcpy(data, srcImage, static_cast<size_t>(imageSize));
    vmaUnmapMemory(mAllocator, stagingBufferAllocation);

    // 创建纹理图像
    imageInfo.usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    VmaAllocationCreateInfo imageAllocInfo = {};
    imageAllocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
    vmaCreateImage(mAllocator, &imageInfo, &imageAllocInfo, &image, &imageAllocation, nullptr);

    VkCommandBuffer commandBuffer = mDevice->BeginSingleTimeCommands();
    {
        // 转换image格式，undefined -> transferDst
        ImageMemoryBarrierInfo imageBarrierInfo{};
        imageBarrierInfo.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageBarrierInfo.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        imageBarrierInfo.srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        imageBarrierInfo.srcAccessMask = 0;
        imageBarrierInfo.dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        imageBarrierInfo.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        mDevice->AddCmdPipelineBarrier(commandBuffer, image, VK_IMAGE_ASPECT_COLOR_BIT, imageBarrierInfo);

        // 拷贝数据，从buffer到image
        VkBufferImageCopy region{};
        region.bufferOffset = 0;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;
        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = 1;
        region.imageOffset = { 0, 0, 0 };
        region.imageExtent = imageInfo.extent;
        vkCmdCopyBufferToImage(commandBuffer, stagingBuffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

        // 转换image格式，transferDst -> shaderReadOnly
        imageBarrierInfo.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        imageBarrierInfo.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageBarrierInfo.srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        imageBarrierInfo.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        imageBarrierInfo.dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        imageBarrierInfo.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        mDevice->AddCmdPipelineBarrier(commandBuffer, image, VK_IMAGE_ASPECT_COLOR_BIT, imageBarrierInfo);
    }
    mDevice->EndSingleTimeCommands(commandBuffer);

    // 清理临时缓冲
    vmaDestroyBuffer(mAllocator, stagingBuffer, stagingBufferAllocation);
}

void BufferCreator::CreateTexturesFromSrcData(std::vector<VkImageCreateInfo>& imageInfos, std::vector<StbImageBuffer>& imageDataList,
    std::vector<VkImage>& images, VkDeviceMemory& imageMemory)
{
    VkDeviceSize maxImageSize = 0;
    for (int i = 0; i < imageDataList.size(); i++) {
        maxImageSize = std::max(maxImageSize, imageDataList[i].size);
    }

    // 创建临时缓冲
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    CreateBuffer(maxImageSize,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        stagingBuffer, stagingBufferMemory);

    // create images
    for (int i = 0; i < imageInfos.size(); i++) {
        imageInfos[i].usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    }
    CreateImages(imageInfos, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, images, imageMemory);

    // copy data
    for (int i = 0; i < imageDataList.size(); i++) {
        // 图像数据拷贝到临时缓冲
        void* data;
        vkMapMemory(mDevice->Get(), stagingBufferMemory, 0, imageDataList[i].size, 0, &data);
        memcpy(data, imageDataList[i].pixels, static_cast<size_t>(imageDataList[i].size));
        vkUnmapMemory(mDevice->Get(), stagingBufferMemory);

        VkCommandBuffer commandBuffer = mDevice->BeginSingleTimeCommands();
        {
            // 转换image格式，undefined -> transferDst
            ImageMemoryBarrierInfo imageBarrierInfo{};
            imageBarrierInfo.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            imageBarrierInfo.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            imageBarrierInfo.srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            imageBarrierInfo.srcAccessMask = 0;
            imageBarrierInfo.dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            imageBarrierInfo.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            mDevice->AddCmdPipelineBarrier(commandBuffer, images[i], VK_IMAGE_ASPECT_COLOR_BIT, imageBarrierInfo);

            // 拷贝数据，从buffer到image
            VkBufferImageCopy region{};
            region.bufferOffset = 0;
            region.bufferRowLength = 0;
            region.bufferImageHeight = 0;
            region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            region.imageSubresource.mipLevel = 0;
            region.imageSubresource.baseArrayLayer = 0;
            region.imageSubresource.layerCount = 1;
            region.imageOffset = { 0, 0, 0 };
            region.imageExtent = imageInfos[i].extent;
            vkCmdCopyBufferToImage(commandBuffer, stagingBuffer, images[i], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

            // 转换image格式，transferDst -> shaderReadOnly
            imageBarrierInfo.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            imageBarrierInfo.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageBarrierInfo.srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            imageBarrierInfo.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            imageBarrierInfo.dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            imageBarrierInfo.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            mDevice->AddCmdPipelineBarrier(commandBuffer, images[i], VK_IMAGE_ASPECT_COLOR_BIT, imageBarrierInfo);
        }
        mDevice->EndSingleTimeCommands(commandBuffer);
    }

    // 清理临时缓冲
    vkDestroyBuffer(mDevice->Get(), stagingBuffer, nullptr);
    vkFreeMemory(mDevice->Get(), stagingBufferMemory, nullptr);
}

void BufferCreator::CreateTexturesFromSrcData(std::vector<VkImageCreateInfo>& imageInfos, std::vector<StbImageBuffer>& imageDataList,
    std::vector<VkImage>& images, std::vector<VmaAllocation>& imageAllocation)
{
    VkDeviceSize maxImageSize = 0;
    for (int i = 0; i < imageDataList.size(); i++) {
        maxImageSize = std::max(maxImageSize, imageDataList[i].size);
    }

    // 创建临时缓冲
    VkBuffer stagingBuffer = VK_NULL_HANDLE;
    VmaAllocation  stagingBufferAllocation = VK_NULL_HANDLE;

    VkBufferCreateInfo stagingBufferInfo = vulkanInitializers::BufferCreateInfo(
        maxImageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
    VmaAllocationCreateInfo stagingBufferAllocInfo = {};
    stagingBufferAllocInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;
    vmaCreateBuffer(mAllocator, &stagingBufferInfo, &stagingBufferAllocInfo,
        &stagingBuffer, &stagingBufferAllocation, nullptr);

    // create images
    images = std::vector<VkImage>(imageInfos.size(), VK_NULL_HANDLE);
    imageAllocation = std::vector<VmaAllocation>(imageInfos.size(), VK_NULL_HANDLE);
    for (int i = 0; i < imageInfos.size(); i++) {
        imageInfos[i].usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;

        VmaAllocationCreateInfo imageAllocInfo = {};
        imageAllocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
        vmaCreateImage(mAllocator, &imageInfos[i], &imageAllocInfo, &images[i], &imageAllocation[i], nullptr);
    }

    // copy data
    for (int i = 0; i < imageDataList.size(); i++) {
        // 图像数据拷贝到临时缓冲
        void* data;
        vmaMapMemory(mAllocator, stagingBufferAllocation, &data);
        memcpy(data, imageDataList[i].pixels, imageDataList[i].size);
        vmaUnmapMemory(mAllocator, stagingBufferAllocation);

        VkCommandBuffer commandBuffer = mDevice->BeginSingleTimeCommands();
        {
            // 转换image格式，undefined -> transferDst
            ImageMemoryBarrierInfo imageBarrierInfo{};
            imageBarrierInfo.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            imageBarrierInfo.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            imageBarrierInfo.srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            imageBarrierInfo.srcAccessMask = 0;
            imageBarrierInfo.dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            imageBarrierInfo.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            mDevice->AddCmdPipelineBarrier(commandBuffer, images[i], VK_IMAGE_ASPECT_COLOR_BIT, imageBarrierInfo);

            // 拷贝数据，从buffer到image
            VkBufferImageCopy region{};
            region.bufferOffset = 0;
            region.bufferRowLength = 0;
            region.bufferImageHeight = 0;
            region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            region.imageSubresource.mipLevel = 0;
            region.imageSubresource.baseArrayLayer = 0;
            region.imageSubresource.layerCount = 1;
            region.imageOffset = { 0, 0, 0 };
            region.imageExtent = imageInfos[i].extent;
            vkCmdCopyBufferToImage(commandBuffer, stagingBuffer, images[i], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

            // 转换image格式，transferDst -> shaderReadOnly
            imageBarrierInfo.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            imageBarrierInfo.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageBarrierInfo.srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            imageBarrierInfo.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            imageBarrierInfo.dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            imageBarrierInfo.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            mDevice->AddCmdPipelineBarrier(commandBuffer, images[i], VK_IMAGE_ASPECT_COLOR_BIT, imageBarrierInfo);
        }
        mDevice->EndSingleTimeCommands(commandBuffer);
    }

    // 清理临时缓冲
    vmaDestroyBuffer(mAllocator, stagingBuffer, stagingBufferAllocation);
}

void BufferCreator::CreateMappedBuffers(std::vector<VkBufferCreateInfo>& bufferInfos, std::vector<VkBuffer>& buffers,
    std::vector<void*>& mappedAddress, VkDeviceMemory& bufferMemory)
{
    if (bufferInfos.empty()) {
        LOGE("bufferINfo empty");
        return;
    }

    // create buffers
    buffers = std::vector<VkBuffer>(bufferInfos.size(), VK_NULL_HANDLE);
    for (int i = 0; i < bufferInfos.size(); i++) {
        if (vkCreateBuffer(mDevice->Get(), &bufferInfos[i], nullptr, &buffers[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create uMvp buffer");
        }
    }

    // get memory requirement
    std::vector<VkMemoryRequirements> memReq(bufferInfos.size());
    for (int i = 0; i < buffers.size(); i++) {
        vkGetBufferMemoryRequirements(mDevice->Get(), buffers[i], &memReq[i]);
    }

    VkDeviceSize totalSize = 0;
    uint32_t memoryType = UINT32_MAX;
    std::vector<VkDeviceSize> offsets(memReq.size(), 0);
    for (int i = 0; i < memReq.size(); i++) {
        VkDeviceSize alignment = memReq[i].alignment;
        offsets[i] = ((totalSize + alignment - 1) / alignment) * alignment;
        totalSize = offsets[i] + memReq[i].size;
        memoryType &= memReq[i].memoryTypeBits;
        LOGI("totalsize=%d offset=%d alignment=%d", totalSize, offsets[i], alignment);
    }

    // create memory
    VkMemoryAllocateInfo allocInfo = vulkanInitializers::MemoryAllocateInfo();
    allocInfo.allocationSize = totalSize;
    allocInfo.memoryTypeIndex = mDevice->GetPhysicalDevice()->FindMemoryType(memoryType,
        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    if (vkAllocateMemory(mDevice->Get(), &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate vertex buffer memory!");
    }

    // bind memory
    for (int i = 0; i < buffers.size(); i++) {
        vkBindBufferMemory(mDevice->Get(), buffers[i], bufferMemory, offsets[i]);
    }

    // map memory
    void* mappedPtr = nullptr;
    vkMapMemory(mDevice->Get(), bufferMemory, 0, allocInfo.allocationSize, 0, &mappedPtr);

    mappedAddress = std::vector<void*>(bufferInfos.size(), nullptr);
    for (int i = 0; i < mappedAddress.size(); i++) {
        mappedAddress[i] = static_cast<void*>(static_cast<uint8_t*>(mappedPtr) + offsets[i]);
        LOGI("mappedAddress=%d", mappedAddress[i]);
    }
}

} // namespace framework