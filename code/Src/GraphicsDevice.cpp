#include "GraphicsDevice.h"
#include "Utils.h"

#include <stdexcept>
#include <vector>
#include <map>
#include <set>
#include <unordered_set>
#include <iostream>

const std::vector<const char*> validationLayers = {
	"VK_LAYER_KHRONOS_validation"
};

const std::vector<const char*> deviceExtensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

namespace render {
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

	void GraphicsDevice::PickPhysicalDevices() {
		// 获取所有物理设备
		uint32_t deviceCount = 0;
		vkEnumeratePhysicalDevices(mInstance, &deviceCount, nullptr);
		if (deviceCount == 0) {
			throw std::runtime_error("failed to find GPUs with Vulkan support!");
		}
		std::vector<VkPhysicalDevice> devices(deviceCount);
		vkEnumeratePhysicalDevices(mInstance, &deviceCount, devices.data());

		// 候选设备map <score, device>
		std::multimap<int, VkPhysicalDevice> candidates;
		for (const auto& device : devices) {
			// 判分
			int score = RateDeviceSuitability(device);
			candidates.insert(std::make_pair(score, device));
		}

		// 选得分最高的设备
		if (candidates.rbegin()->first > 0) {
			mPhysicalDevice = candidates.rbegin()->second;
		}
		else {
			throw std::runtime_error("failed to find a suitable GPU!");
		}

		VkPhysicalDeviceProperties deviceProperties;
		vkGetPhysicalDeviceProperties(mPhysicalDevice, &deviceProperties);
		std::cout << "choose device: " << deviceProperties.deviceName << std::endl;
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
}

