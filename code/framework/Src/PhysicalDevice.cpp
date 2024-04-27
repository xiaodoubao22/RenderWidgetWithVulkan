#include "PhysicalDevice.h"

#include <stdexcept>
#include <iostream>
#include <vector>

#include "Log.h"
#include "Utils.h"
#include "Swapchain.h"
#include "SceneDemoDefs.h"

namespace framework {
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

	ReadRequiredExtensions();
	PickPhysicalDevices();
}

void PhysicalDevice::CleanUp() {
	mQueueFamilyIndices = {};
	mInstance = VK_NULL_HANDLE;
	mSupportedSurface = VK_NULL_HANDLE;
	mPhysicalDevice = VK_NULL_HANDLE;
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

void PhysicalDevice::ReadRequiredExtensions()
{
	mDeviceExtensions = {};

	std::vector<const char*>& demoRequiredExtensions = GetConfig().extension.deviceExtensions;
	mDeviceExtensions.insert(mDeviceExtensions.end(),
		demoRequiredExtensions.begin(), demoRequiredExtensions.end());
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
	LOGI("--------------- pick GPU success ---------------");
	VkPhysicalDeviceProperties deviceProperties;
	vkGetPhysicalDeviceProperties(mPhysicalDevice, &deviceProperties);
	LOGI("device name: %s", deviceProperties.deviceName);
	utils::PrintStringList(mDeviceExtensions, "enable device extensions:");

	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties(mPhysicalDevice, nullptr, &extensionCount, nullptr);
	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(mPhysicalDevice, nullptr, &extensionCount, availableExtensions.data());
	std::vector<const char*> availableExtensionsNames;
	for (VkExtensionProperties& extension : availableExtensions) {
		availableExtensionsNames.push_back(extension.extensionName);
	}
	utils::PrintStringList(availableExtensionsNames, "availiable device extensions:");
	LOGI("------------------------------------------------\n");
}

bool PhysicalDevice::IsDeviceSuatiable(VkPhysicalDevice device) {
	VkPhysicalDeviceProperties deviceProperties;	// name, type, supported Vulkan version
	vkGetPhysicalDeviceProperties(device, &deviceProperties);

	// 显卡类型要求
	if (GetConfig().phisicalDevice.defaultDeviceType != VK_PHYSICAL_DEVICE_TYPE_MAX_ENUM &&	// 等于MAX_ENUM视为无要求
		GetConfig().phisicalDevice.defaultDeviceType != deviceProperties.deviceType) {
		LOGI("%s type not suitable", deviceProperties.deviceName);
		return false;
	}

	// 队列种类不够，不能用
	QueueFamilyIndices indices = FindQueueFamilies(device);
	if (!indices.IsComplete()) {
		LOGI("%s queue family not suitable", deviceProperties.deviceName);
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

	if (!utils::CheckSupported(mDeviceExtensions, availableExtensionsNames)) {
		LOGI("%s extension not suitable", deviceProperties.deviceName);
		return false;
	}

	LOGI("%s suitable", deviceProperties.deviceName);
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
}	// namespace framework