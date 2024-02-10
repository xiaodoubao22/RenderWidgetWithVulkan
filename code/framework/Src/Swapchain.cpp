#include "Swapchain.h"
#include "Utils.h"
#include "WindowTemplate.h"

#include <algorithm>
#include <stdexcept>
#include <iostream>

namespace render {
Swapchain::Swapchain() {

}

Swapchain::~Swapchain() {

}

bool Swapchain::Init(PhysicalDevice* physicalDevice, Device* device, VkExtent2D windowExtent, VkSurfaceKHR surface) {
	if (physicalDevice == nullptr || 
		device == nullptr || 
		!physicalDevice->IsValid() ||
		!device->IsValid()) {
		throw std::runtime_error("can not init swapchain with null/invalid physicalDevice/device !");
	}
	if (mIsInitialized) {
		return false;
	}
		
	// set properties
	mPhysicalDevice = physicalDevice;
	mDevice = device;
	mSurface = surface;

	// create
	CreateSwapChain(windowExtent, VK_NULL_HANDLE);
	CreateImageViews();

	mIsInitialized = true;
	return true;
}

bool Swapchain::CleanUp() {
	if (!mIsInitialized) {
		return false;
	}

	// destroy
	DestroyImageViews();
	vkDestroySwapchainKHR(mDevice->Get(), mSwapChain, nullptr);

	// clear properties
	mPhysicalDevice = nullptr;
	mDevice = nullptr;
	mSurface = VK_NULL_HANDLE;

	mIsInitialized = false;
	return true;
}

bool Swapchain::Recreate(VkExtent2D windowExtent) {
	if (!mIsInitialized) {
		return false;
	}

	DestroyImageViews();
		
	VkSwapchainKHR oldSwapchain = mSwapChain;
	CreateSwapChain(windowExtent, oldSwapchain);
	vkDestroySwapchainKHR(mDevice->Get(), oldSwapchain, nullptr);

	CreateImageViews();

	return true;
}

bool Swapchain::AcquireImage(VkSemaphore imageAvailiableSemaphore, uint32_t& imageIndex) {
	// 从交换链中获取可用的图像，获取到之后触发imageAvailiableSemaphore，表示可以开始画了
	VkResult result = vkAcquireNextImageKHR(mDevice->Get(), mSwapChain, UINT64_MAX, imageAvailiableSemaphore, VK_NULL_HANDLE, &imageIndex);
	
	if (result == VK_SUCCESS || result == VK_SUBOPTIMAL_KHR) {
		return true;
	}
	else if (result == VK_ERROR_OUT_OF_DATE_KHR) {
		return false;
	}
	else {
		// 其他情况认为获取失败
		throw std::runtime_error("failed to acquire swapchain image!");
		return false;
	}
}

bool Swapchain::QueuePresent(uint32_t imageIndex, const std::vector<VkSemaphore>& waitSemaphores) {
	std::vector<VkSwapchainKHR> swapchains = { mSwapChain };

	// 将结果返回给交换链
	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = waitSemaphores.empty() ? 0 : waitSemaphores.size();    // 等待信号个数
	presentInfo.pWaitSemaphores = waitSemaphores.empty() ? nullptr : waitSemaphores.data(); // 等待waitSemaphore，再返回交换链
	presentInfo.swapchainCount = swapchains.size();				// 交换链个数
	presentInfo.pSwapchains = swapchains.data();		// 返回给哪个交换链
	presentInfo.pImageIndices = &imageIndex;	// 返回的图片编号
	presentInfo.pResults = nullptr;				// 各个交换链的返回值（成功与否），此处只用一个交换链，没必要用它
	VkResult result = vkQueuePresentKHR(mDevice->GetPresentQueue(), &presentInfo);

	if (result == VK_SUCCESS) {
		return true;
	}
	else if(result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR){
		return false;
	}
	else {
		throw std::runtime_error("failed to present swap chain image!");
		return false;
	}
}

void Swapchain::CreateSwapChain(VkExtent2D windowExtent, VkSwapchainKHR oldSwapchain) {
	// 获取物理设备支持的交换链配置信息
	SwapChainSupportdetails swapChainSupport = mPhysicalDevice->QuerySwapChainSupport();

	// 选择交换链配置 格式+颜色空间
	VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(swapChainSupport.formats);
	// 选择显示模式
	VkPresentModeKHR presentMode = ChooseSwapPresentMode(swapChainSupport.presentModes);
	// 显示范围 present extent
	VkExtent2D swapchainExtent = ChooseSwapExtent(windowExtent, swapChainSupport.capabilities);
	// 交换链中图像个数
	uint32_t imageCount = std::clamp<uint32_t>(
		setting::swapchainImageCount,
		swapChainSupport.capabilities.minImageCount, 
		swapChainSupport.capabilities.maxImageCount);
	// 模式
	PhysicalDevice::QueueFamilyIndices indices = mPhysicalDevice->GetQueueFamilyIndices();
	std::vector<uint32_t> indicesList(0);
	VkSharingMode sharingMode = ChooseSharingMode(indices, indicesList);

	// 创建交换链
	VkSwapchainCreateInfoKHR createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = mSurface;
	createInfo.minImageCount = imageCount;					// 交换链中图像个数
	createInfo.imageFormat = surfaceFormat.format;			// 格式
	createInfo.imageColorSpace = surfaceFormat.colorSpace;	// 颜色空间
	createInfo.imageExtent = swapchainExtent;						// 图像大小(分辨率)
	createInfo.imageArrayLayers = 1;						// 图像有几层（除非做VR程序，否则是1）
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;	// SwapChain中的图像用作颜色附件
	createInfo.imageSharingMode = sharingMode;
	createInfo.queueFamilyIndexCount = indicesList.size();
	createInfo.pQueueFamilyIndices = indicesList.empty() ? nullptr : indicesList.data();
	createInfo.preTransform = swapChainSupport.capabilities.currentTransform;	//  旋转/镜像 ：不操作
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;	// alpha通道是否与其他窗口混合：否
	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;		// 不关心被（其他窗口）遮挡像素的颜色
	createInfo.oldSwapchain = oldSwapchain;
	if (vkCreateSwapchainKHR(mDevice->Get(), &createInfo, nullptr, &mSwapChain) != VK_SUCCESS) {
		throw std::runtime_error("failed to create swap chain!");
	}

	// 取出交换链中的图像
	vkGetSwapchainImagesKHR(mDevice->Get(), mSwapChain, &mImageCount, nullptr);
	mSwapchainImages.resize(mImageCount);
	vkGetSwapchainImagesKHR(mDevice->Get(), mSwapChain, &mImageCount, mSwapchainImages.data());

	// 保存格式和分辨率信息
	mSwapchainImageFormat = surfaceFormat.format;
	mSwapchainExtent = swapchainExtent;
	mPresentMode = presentMode;
	mImageCount = imageCount;
}

void Swapchain::CreateImageViews() {
	mSwapchainImageViews.clear();
	mSwapchainImageViews.resize(mSwapchainImages.size());
	for (size_t i = 0; i < mSwapchainImages.size(); i++) {
		VkImageViewCreateInfo viewInfo{};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image = mSwapchainImages[i];
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.format = mSwapchainImageFormat;
		viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;

		if (vkCreateImageView(mDevice->Get(), &viewInfo, nullptr, &mSwapchainImageViews[i]) != VK_SUCCESS) {
			throw std::runtime_error("failed to create texture image view!");
		}
	}
}

void Swapchain::DestroyImageViews() {
	for (auto imageView : mSwapchainImageViews) {
		vkDestroyImageView(mDevice->Get(), imageView, nullptr);
	}
	mSwapchainImageViews.clear();
}

VkSurfaceFormatKHR Swapchain::ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
	// 选择合适的surface格式
	for (const auto& availableFormat : availableFormats) {
		// 格式 + 颜色空间
		if (availableFormat.format == setting::swapchainSurfaceFormat.format &&
			availableFormat.colorSpace == setting::swapchainSurfaceFormat.colorSpace) {
			return availableFormat;
		}
	}
	return availableFormats[0];
}

VkPresentModeKHR Swapchain::ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availiablePresentModes) {
	for (const auto& availiablePresentMode : availiablePresentModes) {
		if (availiablePresentMode == setting::presentMode) {
			return availiablePresentMode;
		}
	}
	return VK_PRESENT_MODE_FIFO_KHR;	// 默认
}

VkExtent2D Swapchain::ChooseSwapExtent(VkExtent2D windowExtent, const VkSurfaceCapabilitiesKHR& capabilities) {
	if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
		return capabilities.currentExtent;
	}
	else {
		// 分辨率无效，重新处理
		windowExtent.width = std::clamp(windowExtent.width,
			capabilities.minImageExtent.width,
			capabilities.maxImageExtent.width);
		windowExtent.height = std::clamp(windowExtent.height,
			capabilities.minImageExtent.height,
			capabilities.maxImageExtent.height);
		return windowExtent;
	}
}

VkSharingMode Swapchain::ChooseSharingMode(const PhysicalDevice::QueueFamilyIndices& indices, std::vector<uint32_t>& indicesList) {
	indicesList.clear();
	if (indices.graphicsFamily != indices.presentFamily) {
		indicesList = { indices.graphicsFamily.value(), indices.presentFamily.value() };
		return VK_SHARING_MODE_CONCURRENT;	// 图像可以同时被两个队列族使用
	}
	else {
		return VK_SHARING_MODE_EXCLUSIVE;	// 图像只能被一个队列族使用，换队列族需要显式转移所有权
	}
}
}