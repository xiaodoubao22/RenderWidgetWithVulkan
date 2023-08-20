#include "DebugUtils.h"
#include "Utils.h"

#include <iostream>
#include <sstream>

namespace render {

	DebugUtils::DebugUtils() {

	}

	DebugUtils::~DebugUtils() {

	}

	DebugUtils& DebugUtils::GetInstance() {
		static DebugUtils instance;
		return instance;
	}

	void DebugUtils::Setup(VkInstance instance) {
		if (mIsSetup || !setting::enableValidationLayer) {
			return;
		}

		// load functions
		mPfVkCreateDebugUtilsMessengerEXT = 
			(PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
		mPfVkDestroyDebugUtilsMessengerEXT = 
			(PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
		if (mPfVkCreateDebugUtilsMessengerEXT == nullptr ||
			mPfVkDestroyDebugUtilsMessengerEXT == nullptr) {
			throw std::runtime_error("not found debug utils functions!");
		}

		// create
		VkDebugUtilsMessengerCreateInfoEXT createInfo{};
		PopulateDebugMessengerCreateInfo(createInfo);
		if (mPfVkCreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &mDebugUtilsMessenger) != VK_SUCCESS) {
			throw std::runtime_error("failed to set up debug messenger!");
		}

		mIsSetup = true;
		return;
	}

	void DebugUtils::Destroy(VkInstance instance) {
		if (!mIsSetup || !setting::enableValidationLayer) {
			return;
		}

		mPfVkDestroyDebugUtilsMessengerEXT(instance, mDebugUtilsMessenger, nullptr);

		mIsSetup = false;
		return;
	}

    VKAPI_ATTR VkBool32 VKAPI_CALL DebugUtils::DebugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData) {
		// Select prefix depending on flags passed to the callback
		std::string prefix("");

		if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT) {
			prefix = "VERBOSE: ";
		}
		else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT) {
			prefix = "INFO: ";
		}
		else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
			prefix = "WARNING: ";
		}
		else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
			prefix = "ERROR: ";
		}

		// Display message to default output (console/logcat)
		std::stringstream debugMessage;
		debugMessage << prefix << "[" << pCallbackData->messageIdNumber << "][" << pCallbackData->pMessageIdName << "] : " << pCallbackData->pMessage;

		if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
			std::cerr << debugMessage.str() << "\n";
		}
		else {
			//std::cout << debugMessage.str() << "\n";
		}
		fflush(stdout);

		// The return value of this callback controls whether the Vulkan call that caused the validation message will be aborted or not
		// We return VK_FALSE as we DON'T want Vulkan calls that cause a validation message to abort
		// If you instead want to have calls abort, pass in VK_TRUE and the function will return VK_ERROR_VALIDATION_FAILED_EXT 
		return VK_FALSE;
    }

	void DebugUtils::PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
		createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		createInfo.messageSeverity = 
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | 
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | 
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		createInfo.messageType = 
			VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | 
			VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | 
			VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		createInfo.pfnUserCallback = DebugCallback;
	}
}
