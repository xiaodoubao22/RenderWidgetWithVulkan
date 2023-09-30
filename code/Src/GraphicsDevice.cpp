#include "GraphicsDevice.h"
#include "Utils.h"

#include <stdexcept>
#include <vector>
#include <map>
#include <set>
#include <unordered_set>
#include <iostream>
#include <bitset>

namespace render {
	VkResult GetPhysicalDeviceFragmentShadingRatesKHR(VkInstance instance, VkPhysicalDevice physicalDevice,
		uint32_t* pFragmentShadingRateCount, VkPhysicalDeviceFragmentShadingRateKHR* pFragmentShadingRates) {
		auto func = (PFN_vkGetPhysicalDeviceFragmentShadingRatesKHR)vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceFragmentShadingRatesKHR");
		if (func != nullptr) {
			return func(physicalDevice, pFragmentShadingRateCount, pFragmentShadingRates);
		}
		else {
			return VK_ERROR_EXTENSION_NOT_PRESENT;
		}
	}

    GraphicsDevice::GraphicsDevice() {

    }

    GraphicsDevice::~GraphicsDevice() {

    }

    void GraphicsDevice::Init(VkInstance instance, VkSurfaceKHR supportedSurface) {
		if (mIsInitialized == true) {
			return;
		}
		if (instance == VK_NULL_HANDLE) {
			throw std::runtime_error("can not init graphics device with a null instance!");
		}
		if (supportedSurface == VK_NULL_HANDLE) {
			throw std::runtime_error("can not init graphics device with a null surface!");
		}
		mInstance = instance;
		mSupportedSurface = supportedSurface;

		PickPhysicalDevices();
		CreateLogicalDevice();
		CreateCommandPool();

		mIsInitialized = true;
    }

    void GraphicsDevice::CleanUp() {
		vkDestroyCommandPool(mDevice, mCommandPoolOfGraphics, nullptr);
		vkDestroyDevice(mDevice, nullptr);

		mIsInitialized = false;
    }

	SwapChainSupportdetails GraphicsDevice::QuerySwapChainSupport() {
		if (!mIsInitialized) {
			return {};
		}

		// 获取容量信息
		SwapChainSupportdetails details;	// capabilities formats presentModes
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(mPhysicalDevice, mSupportedSurface, &details.capabilities);

		// 获取格式信息
		uint32_t formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(mPhysicalDevice, mSupportedSurface, &formatCount, nullptr);
		if (formatCount != 0) {
			details.formats.resize(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(mPhysicalDevice, mSupportedSurface, &formatCount, details.formats.data());
		}

		// 获取presentation modes
		uint32_t presentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(mPhysicalDevice, mSupportedSurface, &presentModeCount, nullptr);
		if (presentModeCount != 0) {
			details.presentModes.resize(presentModeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(mPhysicalDevice, mSupportedSurface, &presentModeCount, details.presentModes.data());
		}

		details.isValid = true;
		return details;
	}

	VkFormat GraphicsDevice::FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) {
		if (mPhysicalDevice == VK_NULL_HANDLE) {
			throw std::runtime_error("mPhysicalDevice is null");
		}
		for (VkFormat format : candidates) {
			VkFormatProperties props;
			vkGetPhysicalDeviceFormatProperties(mPhysicalDevice, format, &props);
			if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
				return format;
			}
			else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
				return format;
			}
		}
		throw std::runtime_error("failed to find supported format!");
	}

	uint32_t GraphicsDevice::FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
		VkPhysicalDeviceMemoryProperties memProperties;
		vkGetPhysicalDeviceMemoryProperties(mPhysicalDevice, &memProperties);
		for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
			bool typeAviliableFlag = typeFilter & (1 << i);
			bool propertiesSupportFlag = (memProperties.memoryTypes[i].propertyFlags & properties) == properties;
			if (typeAviliableFlag && propertiesSupportFlag) {
				return i;
			}
		}
		throw std::runtime_error("failed to find suitable memory type!");
	}

	void GraphicsDevice::CreateImage(VkImageCreateInfo* pImageInfo, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory) {
		// 创建VkImage
		if (vkCreateImage(mDevice, pImageInfo, nullptr, &image) != VK_SUCCESS) {
			throw std::runtime_error("failed to create image!");
		}

		// 为VkImage分配显存
		VkMemoryRequirements memRequirements;
		vkGetImageMemoryRequirements(mDevice, image, &memRequirements);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, properties);
		if (vkAllocateMemory(mDevice, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate image memory");
		}

		vkBindImageMemory(mDevice, image, imageMemory, 0);
	}

	VkCommandBuffer GraphicsDevice::CreateCommandBuffer(VkCommandBufferLevel level) {
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

	void GraphicsDevice::FreeCommandBuffer(VkCommandBuffer commandBuffer) {
		// 回收命令缓冲
		vkFreeCommandBuffers(mDevice, mCommandPoolOfGraphics, 1, &commandBuffer);
	}

	void GraphicsDevice::CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
		VkBuffer& buffer, VkDeviceMemory& bufferMemory) {
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
		allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, properties);
		if (vkAllocateMemory(mDevice, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate vertex buffer memory!");
		}

		// 显存绑定到缓冲区
		vkBindBufferMemory(mDevice, buffer, bufferMemory, 0);
	}

	VkCommandBuffer GraphicsDevice::BeginSingleTimeCommands() {
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

	void GraphicsDevice::EndSingleTimeCommands(VkCommandBuffer commandBuffer) {
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

	void GraphicsDevice::CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
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

	void GraphicsDevice::CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) {
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

	void GraphicsDevice::TransitionImageLayout(VkImage image, VkImageAspectFlags aspectMask, const ImageMemoryBarrierInfo& barrierInfo, uint32_t mipLevels) {
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

	void GraphicsDevice::PickPhysicalDevices() {
		// 获取所有物理设备
		uint32_t deviceCount = 0;
		vkEnumeratePhysicalDevices(mInstance, &deviceCount, nullptr);
		if (deviceCount == 0) {
			throw std::runtime_error("failed to find GPUs with Vulkan support!");
		}
		std::vector<VkPhysicalDevice> devices(deviceCount);
		vkEnumeratePhysicalDevices(mInstance, &deviceCount, devices.data());
		
		// 选择符合要求的设备
		mPhysicalDevice = VK_NULL_HANDLE;
		for (const auto& candidateDevice : devices) {
			if (IsDeviceSuatiable(candidateDevice)) {
				mPhysicalDevice = candidateDevice;
				break;
			}
		}
		
		if (mPhysicalDevice == VK_NULL_HANDLE) {
			throw std::runtime_error("failed to find a suitable GPU!");
		}
		VkPhysicalDeviceProperties deviceProperties;
		vkGetPhysicalDeviceProperties(mPhysicalDevice, &deviceProperties);

		// 输出选择结果
		std::cout << "--------------- pick GPU success ---------------\n";
		std::cout << "device name: " << deviceProperties.deviceName << std::endl;
		utils::PrintStringList(consts::deviceExtensions, "enable device extensions:");

		uint32_t extensionCount;
		vkEnumerateDeviceExtensionProperties(mPhysicalDevice, nullptr, &extensionCount, nullptr);
		std::vector<VkExtensionProperties> availableExtensions(extensionCount);
		vkEnumerateDeviceExtensionProperties(mPhysicalDevice, nullptr, &extensionCount, availableExtensions.data());
		std::vector<const char*> availableExtensionsNames;
		for (VkExtensionProperties& extension : availableExtensions) {
			availableExtensionsNames.push_back(extension.extensionName);
		}
		utils::PrintStringList(availableExtensionsNames, "availiable device extensions:");
		std::cout << "------------------------------------------------\n\n";

		// ******************查询VRS特性，TODO 移到选择GPU的逻辑中 *******************
		// feature
		VkPhysicalDeviceFragmentShadingRateFeaturesKHR physicalDeviceFSRFeatures{
			VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADING_RATE_FEATURES_KHR 
		};
		VkPhysicalDeviceFeatures2 physicalDeviceFeatures2{};
		physicalDeviceFeatures2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
		physicalDeviceFeatures2.pNext = &physicalDeviceFSRFeatures;
		vkGetPhysicalDeviceFeatures2(mPhysicalDevice, &physicalDeviceFeatures2);

		std::cout << "physicalDeviceFSRFeatures : " 
			<< physicalDeviceFSRFeatures.attachmentFragmentShadingRate << " "
			<< physicalDeviceFSRFeatures.pipelineFragmentShadingRate << " "
			<< physicalDeviceFSRFeatures.primitiveFragmentShadingRate << std::endl;

		// properties
		VkPhysicalDeviceFragmentShadingRatePropertiesKHR physicalDeviceFSRProperties{
			VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADING_RATE_PROPERTIES_KHR
		};
		VkPhysicalDeviceProperties2 physicalDeviceProperties2{};
		physicalDeviceProperties2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
		physicalDeviceProperties2.pNext = &physicalDeviceFSRProperties;
		vkGetPhysicalDeviceProperties2(mPhysicalDevice, &physicalDeviceProperties2);
		std::cout << "physicalDeviceFSRProperties : "
			<< "(" << physicalDeviceFSRProperties.minFragmentShadingRateAttachmentTexelSize.width << ", "
			<< physicalDeviceFSRProperties.minFragmentShadingRateAttachmentTexelSize.height << ") "
			<< "(" << physicalDeviceFSRProperties.maxFragmentShadingRateAttachmentTexelSize.width << ", "
			<< physicalDeviceFSRProperties.maxFragmentShadingRateAttachmentTexelSize.height << ") "
			<< "(" << physicalDeviceFSRProperties.maxFragmentSizeAspectRatio << ")\n";

		// availiable shading rate
		uint32_t availiableShadingRateSize = 0;
		std::vector<VkPhysicalDeviceFragmentShadingRateKHR> availiableShadingRate(0);
		VkResult res = GetPhysicalDeviceFragmentShadingRatesKHR(mInstance, mPhysicalDevice, &availiableShadingRateSize, nullptr);
		if (availiableShadingRateSize != 0 && res == VK_SUCCESS) {
			availiableShadingRate.resize(availiableShadingRateSize, { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADING_RATE_KHR });
			GetPhysicalDeviceFragmentShadingRatesKHR(mInstance, mPhysicalDevice, &availiableShadingRateSize, availiableShadingRate.data());
		}
		
		for (VkPhysicalDeviceFragmentShadingRateKHR& shadingRate : availiableShadingRate) {
			std::cout << std::bitset<sizeof(VkSampleCountFlags) * 8>(shadingRate.sampleCounts)
				<< " (" << shadingRate.fragmentSize.width << "," << shadingRate.fragmentSize.height << ")" << std::endl;
		}
		// **********************************************************************
	}

	void GraphicsDevice::CreateLogicalDevice() {
		// ------ fill queueCreateInfo -------
		mQueueFamilyIndices = FindQueueFamilies(mPhysicalDevice);
 
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
		createInfo.enabledExtensionCount = consts::deviceExtensions.size();	// 设备支持的拓展：交换链
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

		if (vkCreateDevice(mPhysicalDevice, &createInfo, nullptr, &mDevice) != VK_SUCCESS) {
			throw std::runtime_error("failed to create logical device!");
		}

		// 从逻辑设备中取出图形队列（根据队列族编号）
		vkGetDeviceQueue(mDevice, mQueueFamilyIndices.graphicsFamily.value(), 0, &mGraphicsQueue);
		vkGetDeviceQueue(mDevice, mQueueFamilyIndices.presentFamily.value(), 0, &mPresentQueue);
	}

	void GraphicsDevice::CreateCommandPool() {
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

	int GraphicsDevice::RateDeviceSuitability(VkPhysicalDevice device) {
		VkPhysicalDeviceProperties deviceProperties;	// name, type, supported Vulkan version
		vkGetPhysicalDeviceProperties(device, &deviceProperties);
		//VkPhysicalDeviceFeatures devicesFeatures;	// texure compression, 64 bit float, multi viewport rendering(for VR)
		//vkGetPhysicalDeviceFeatures(device, &devicesFeatures);

		int score = 0;

		// 显卡类型要求
		if (setting::defaultDeviceType != VK_PHYSICAL_DEVICE_TYPE_MAX_ENUM &&
			setting::defaultDeviceType == deviceProperties.deviceType) {
			score += 1000;
		}

		// 队列种类不够，不能用
		QueueFamilyIndices indices = FindQueueFamilies(device);
		if (!indices.IsComplete()) {
			return 0;
		}

		return score;
	}

	bool GraphicsDevice::IsDeviceSuatiable(VkPhysicalDevice device) {
		VkPhysicalDeviceProperties deviceProperties;	// name, type, supported Vulkan version
		vkGetPhysicalDeviceProperties(device, &deviceProperties);
		std::cout << deviceProperties.deviceName;

		// 显卡类型要求
		if (setting::defaultDeviceType != VK_PHYSICAL_DEVICE_TYPE_MAX_ENUM &&	// 等于MAX_ENUM视为无要求
			setting::defaultDeviceType != deviceProperties.deviceType) {
			std::cout <<" type not suatiable \n";
			return false;
		}

		// 队列种类不够，不能用
		QueueFamilyIndices indices = FindQueueFamilies(device);
		if (!indices.IsComplete()) {
			std::cout << " queue family not suatiable \n";
			return false;
		}

		// 拓展是否支持
		uint32_t extensionCount;
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);
		std::vector<VkExtensionProperties> availableExtensions(extensionCount);
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

		std::vector<const char*> availableExtensionsNames;
		for (VkExtensionProperties& extension : availableExtensions) {
			availableExtensionsNames.push_back(extension.extensionName);
		}

		if (!utils::CheckSupported(consts::deviceExtensions, availableExtensionsNames)) {
			std::cout << " extension not suatiable \n";
			return false;
		}

		std::cout << " suatiable \n";
		return true;
	}

	QueueFamilyIndices GraphicsDevice::FindQueueFamilies(VkPhysicalDevice device) {
		QueueFamilyIndices indices{};

		// 读取所有queueFamily
		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

		// 寻找想要的queueFamily，记录索引
		int i = 0;
		for (const auto& queueFamily : queueFamilies) {
			// 找支持图形的队列
			if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
				indices.graphicsFamily = i;
			}
			// 找支持surface的队列
			VkBool32 presentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(device, i, mSupportedSurface, &presentSupport);
			if (presentSupport) {
				indices.presentFamily = i;
			}
			if (indices.IsComplete()) break;
			i++;
		}

		return indices;
	}

	void GraphicsDevice::GetAvailiableExtensionsNames(VkPhysicalDevice device, std::vector<const char*>& extensionsNames) {
		uint32_t extensionCount;
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);
		std::vector<VkExtensionProperties> availableExtensions(extensionCount);
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

		extensionsNames.clear();
		for (VkExtensionProperties& extension : availableExtensions) {
			extensionsNames.push_back(extension.extensionName);
		}

	}
}

