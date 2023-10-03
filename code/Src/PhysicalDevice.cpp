#include "PhysicalDevice.h"

#include <stdexcept>
#include <iostream>
#include <vector>
#include <bitset>

#include "Utils.h"
#include "Swapchain.h"

namespace render {
    PhysicalDevice::PhysicalDevice() {

    }

    PhysicalDevice::~PhysicalDevice() {

    }

    void PhysicalDevice::Init(VkInstance instance, VkSurfaceKHR supportedSurface) {
		if (mPhysicalDevice != VK_NULL_HANDLE) {
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
    }

	void PhysicalDevice::CleanUp() {
		mQueueFamilyIndices = {};
		mInstance = VK_NULL_HANDLE;
		mSupportedSurface = VK_NULL_HANDLE;
		mPhysicalDevice = VK_NULL_HANDLE;
	}

	void PhysicalDevice::SetAdditionalSuiatbleTestFunction(const std::function<bool(VkPhysicalDevice)>& func) {
		mAdditionalSuiatbleTest = func;
	}

	SwapChainSupportdetails PhysicalDevice::QuerySwapChainSupport() {
		if (mPhysicalDevice == VK_NULL_HANDLE) {
			std::cout << "can not query swapchain support with a invalid physical device\n";
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

	VkFormat PhysicalDevice::FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) {
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

	uint32_t PhysicalDevice::FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
		if (mPhysicalDevice == VK_NULL_HANDLE) {
			throw std::runtime_error("mPhysicalDevice is null");
		}
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

	void PhysicalDevice::PickPhysicalDevices() {
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
		mQueueFamilyIndices = FindQueueFamilies(mPhysicalDevice);	// 保存队列族编号

		// 输出选择结果
		std::cout << "--------------- pick GPU success ---------------\n";
		VkPhysicalDeviceProperties deviceProperties;
		vkGetPhysicalDeviceProperties(mPhysicalDevice, &deviceProperties);
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

	bool PhysicalDevice::IsDeviceSuatiable(VkPhysicalDevice device) {
		VkPhysicalDeviceProperties deviceProperties;	// name, type, supported Vulkan version
		vkGetPhysicalDeviceProperties(device, &deviceProperties);

		// 显卡类型要求
		if (setting::defaultDeviceType != VK_PHYSICAL_DEVICE_TYPE_MAX_ENUM &&	// 等于MAX_ENUM视为无要求
			setting::defaultDeviceType != deviceProperties.deviceType) {
			std::cout << deviceProperties.deviceName << " type not suitable \n";
			return false;
		}

		// 队列种类不够，不能用
		QueueFamilyIndices indices = FindQueueFamilies(device);
		if (!indices.IsComplete()) {
			std::cout << deviceProperties.deviceName << " queue family not suitable \n";
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
			std::cout << deviceProperties.deviceName << " extension not suitable \n";
			return false;
		}

		if (mAdditionalSuiatbleTest != nullptr && !mAdditionalSuiatbleTest(device)) {
			return false;
		}

		std::cout << deviceProperties.deviceName << " suitable \n";
		return true;
	}

	PhysicalDevice::QueueFamilyIndices PhysicalDevice::FindQueueFamilies(VkPhysicalDevice device) {
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