#include "Device.h"

#include <stdexcept>
#include <set>

#include "Utils.h"

namespace render {
    void Device::Init(PhysicalDevice* physicalDevice) {
		if (physicalDevice == nullptr && !physicalDevice->IsValid()) {
			throw std::runtime_error("can not init graphics device with a null or invalid physicalDevice!");
			return;
		}

        mPhysicalDevice = physicalDevice;

		CreateLogicalDevice();
		CreateCommandPool();
    }

    void Device::CleanUp() {
        vkDestroyCommandPool(mDevice, mCommandPoolOfGraphics, nullptr);
        vkDestroyDevice(mDevice, nullptr);
		mPhysicalDevice = nullptr;
    }

	void Device::CreateImage(VkImageCreateInfo* pImageInfo, VkMemoryPropertyFlags properties,
		VkImage& image, VkDeviceMemory& imageMemory) {
		if (mPhysicalDevice == nullptr || mDevice == nullptr) {
			throw std::runtime_error("mPhysicalDevice/mDevice is null!");
		}

		if (vkCreateImage(mDevice, pImageInfo, nullptr, &image) != VK_SUCCESS) {    // 创建VkImage
			throw std::runtime_error("failed to create image!");
		}

		// 为VkImage分配显存
		VkMemoryRequirements memRequirements;
		vkGetImageMemoryRequirements(mDevice, image, &memRequirements);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = mPhysicalDevice->FindMemoryType(memRequirements.memoryTypeBits, properties);
		if (vkAllocateMemory(mDevice, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate image memory");
		}

		vkBindImageMemory(mDevice, image, imageMemory, 0);
	}

	VkCommandBuffer Device::CreateCommandBuffer(VkCommandBufferLevel level) {
		if (mDevice == nullptr) {
			throw std::runtime_error("mDevice is null!");
		}

		VkCommandBufferAllocateInfo allocateInfo{};
		allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocateInfo.commandPool = mCommandPoolOfGraphics;
		allocateInfo.level = level;
		allocateInfo.commandBufferCount = 1;

		VkCommandBuffer commandBuffer = VK_NULL_HANDLE;
		if (vkAllocateCommandBuffers(mDevice, &allocateInfo, &commandBuffer) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate command buffers!");
		}
		return commandBuffer;
	}

	void Device::FreeCommandBuffer(VkCommandBuffer commandBuffer) {
		if (mDevice == nullptr) {
			throw std::runtime_error("mDevice is null!");
		}
		vkFreeCommandBuffers(mDevice, mCommandPoolOfGraphics, 1, &commandBuffer);
	}

	void Device::CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
		VkBuffer& buffer, VkDeviceMemory& bufferMemory) {
		if (mPhysicalDevice == nullptr || mDevice == nullptr) {
			throw std::runtime_error("mPhysicalDevice/mDevice is null!");
		}

		// 创建缓冲区
		VkBufferCreateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = size; // 缓冲区大小
		bufferInfo.usage = usage;	 // 缓冲区用途
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;		 // 指定独占模式，因为只有一个队列（图形队列）需要用它
		bufferInfo.flags = 0;
		if (vkCreateBuffer(mDevice, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
			throw std::runtime_error("failed to create vertex buffer");
		}

		// 该buffer对显存的要求
		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(mDevice, buffer, &memRequirements);

		// 申请显存
		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;		// 显存大小
		allocInfo.memoryTypeIndex = mPhysicalDevice->FindMemoryType(memRequirements.memoryTypeBits, properties);
		if (vkAllocateMemory(mDevice, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate vertex buffer memory!");
		}

		// 显存绑定到缓冲区
		vkBindBufferMemory(mDevice, buffer, bufferMemory, 0);
	}

	VkCommandBuffer Device::BeginSingleTimeCommands() {
		if (mDevice == nullptr) {
			throw std::runtime_error("mDevice is null!");
		}

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = mCommandPoolOfGraphics;
		allocInfo.commandBufferCount = 1;

		// 申请一个命令缓冲
		VkCommandBuffer commandBuffer;
		vkAllocateCommandBuffers(mDevice, &allocInfo, &commandBuffer);

		// 启动命令缓冲
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;	// 只提交一次
		vkBeginCommandBuffer(commandBuffer, &beginInfo);

		return commandBuffer;
	}

	void Device::EndSingleTimeCommands(VkCommandBuffer commandBuffer) {
		if (mDevice == nullptr) {
			throw std::runtime_error("mDevice is null!");
		}

		// 结束
		vkEndCommandBuffer(commandBuffer);

		// 提交
		// 此处提交给了图形队列，因为图形队列肯定能用于传输
		// TODO：尝试获取一个新的、支持传输的队列
		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;
		vkQueueSubmit(mGraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(mGraphicsQueue);

		// 回收命令缓冲
		vkFreeCommandBuffers(mDevice, mCommandPoolOfGraphics, 1, &commandBuffer);
	}

	void Device::CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
		// 申请并启动一个命令缓冲
		VkCommandBuffer commandBuffer = BeginSingleTimeCommands();

		// 写入复制命令
		VkBufferCopy copyRegion{};
		copyRegion.srcOffset = 0;
		copyRegion.dstOffset = 0;
		copyRegion.size = size;
		vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

		// 提交命令缓冲
		EndSingleTimeCommands(commandBuffer);
	}

	void Device::CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) {
		VkCommandBuffer commandBuffer = BeginSingleTimeCommands();

		VkBufferImageCopy region{};
		region.bufferOffset = 0;
		region.bufferRowLength = 0;
		region.bufferImageHeight = 0;

		region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		region.imageSubresource.mipLevel = 0;
		region.imageSubresource.baseArrayLayer = 0;
		region.imageSubresource.layerCount = 1;

		region.imageOffset = { 0, 0, 0 };
		region.imageExtent = { width, height, 1 };

		vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

		EndSingleTimeCommands(commandBuffer);
	}

	void Device::TransitionImageLayout(VkImage image, VkImageAspectFlags aspectMask,
		const ImageMemoryBarrierInfo& barrierInfo, uint32_t mipLevels) {
		VkCommandBuffer commandBuffer = BeginSingleTimeCommands();

		VkImageMemoryBarrier barrier{};	// 可用来转换Image格式，也可用来转换队列组所有权
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.srcAccessMask = barrierInfo.srcAccessMask;
		barrier.dstAccessMask = barrierInfo.dstAccessMask;
		barrier.oldLayout = barrierInfo.oldLayout;
		barrier.newLayout = barrierInfo.newLayout;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;	// 如果不用来转换队列族所有权，必须设置为ignored(非默认)
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = image;
		barrier.subresourceRange.aspectMask = aspectMask;
		barrier.subresourceRange.baseMipLevel = 0;		// mipmap级别
		barrier.subresourceRange.levelCount = mipLevels;
		barrier.subresourceRange.baseArrayLayer = 0;	// 图像数组
		barrier.subresourceRange.layerCount = 1;

		vkCmdPipelineBarrier(commandBuffer,
			barrierInfo.srcStage, barrierInfo.dstStage,			// srcStageMask    dstStageMask
			0,				// dependencyFlags
			0, nullptr,		// memory barrier
			0, nullptr,		// buffer memory barrier
			1, &barrier		// image memory barrier
		);

		EndSingleTimeCommands(commandBuffer);
	}

    void Device::CreateLogicalDevice() {
		if (mPhysicalDevice == nullptr) {
			throw std::runtime_error("mPhysicalDevice is null!");
		}

		// ------ fill queueCreateInfo -------
		mQueueFamilyIndices = mPhysicalDevice->GetQueueFamilyIndices();

		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
		std::set<uint32_t> uniqueQueueFamilies = {
			mQueueFamilyIndices.graphicsFamily.value(),
			mQueueFamilyIndices.presentFamily.value() };	// 用set去重

		float queuePriority = 1.0f;		// 优先级
		for (uint32_t queueFamily : uniqueQueueFamilies) {
			VkDeviceQueueCreateInfo queueCreateInfo{};
			queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCreateInfo.queueFamilyIndex = queueFamily;		// 队列编号
			queueCreateInfo.queueCount = 1;						// 1个队列
			queueCreateInfo.pQueuePriorities = &queuePriority;	// 优先级
			queueCreateInfos.push_back(queueCreateInfo);
		}

		// ------ fill device features -------
		VkPhysicalDeviceFeatures deviceFeatures{};
		deviceFeatures.samplerAnisotropy = VK_TRUE;	// 各向异性滤波，暂时不用

		// ------ create device -------
		VkDeviceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());	// 两个命令队列
		createInfo.pQueueCreateInfos = queueCreateInfos.data();								// 两个命令队列
		createInfo.pEnabledFeatures = &deviceFeatures;
		createInfo.enabledExtensionCount = consts::deviceExtensions.size();	// 设备支持的拓展：交换链等
		createInfo.ppEnabledExtensionNames = consts::deviceExtensions.data();
		if (setting::enableValidationLayer) {
			createInfo.enabledLayerCount = consts::validationLayers.size();
			createInfo.ppEnabledLayerNames = consts::validationLayers.data();
		}
		else {
			createInfo.enabledLayerCount = 0;
		}

		VkPhysicalDeviceFragmentShadingRateFeaturesKHR shadingRateCreateInfo{};
		shadingRateCreateInfo.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADING_RATE_FEATURES_KHR;
		shadingRateCreateInfo.pipelineFragmentShadingRate = true;
		shadingRateCreateInfo.attachmentFragmentShadingRate = false;
		shadingRateCreateInfo.primitiveFragmentShadingRate = false;

		createInfo.pNext = &shadingRateCreateInfo;

		if (vkCreateDevice(mPhysicalDevice->Get(), &createInfo, nullptr, &mDevice) != VK_SUCCESS) {
			throw std::runtime_error("failed to create logical device!");
		}

		// 从逻辑设备中取出图形队列（根据队列族编号）
		vkGetDeviceQueue(mDevice, mQueueFamilyIndices.graphicsFamily.value(), 0, &mGraphicsQueue);
		vkGetDeviceQueue(mDevice, mQueueFamilyIndices.presentFamily.value(), 0, &mPresentQueue);
    }

    void Device::CreateCommandPool() {
		if (mDevice == nullptr) {
			throw std::runtime_error("mDevice is null!");
		}

		// CommandPool用来管理Commandbuffer，可以从中申请CommandBuffer
		// 一个CommandPool对应一种队列族，只能提供一种CommandBuffer（要么绘图用，要么显示用），这里设为绘图用（poolInfo.queueFamilyIndex）
		// 允许CommandPool中的CommandBuffer单独被记录、reset，设置(poolInfo.flags)
		VkCommandPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		poolInfo.queueFamilyIndex = mQueueFamilyIndices.graphicsFamily.value();

		if (vkCreateCommandPool(mDevice, &poolInfo, nullptr, &mCommandPoolOfGraphics) != VK_SUCCESS) {
			throw std::runtime_error("failed to create command pool!");
		}
    }
}

