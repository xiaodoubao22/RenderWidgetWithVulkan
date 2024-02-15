#include "WindowTemplate.h"
#include <stdexcept>
#include <iostream>
#include "Utils.h"

namespace window {
WindowTemplate::WindowTemplate(bool resizable) {
    // 初始化GLFW窗口
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);	// 不要创建OpenGL上下文
    glfwWindowHint(GLFW_RESIZABLE, resizable);		// 禁止调整窗口大小
    mWindow = glfwCreateWindow(setting::WINDOW_WIDTH, setting::WINDOW_HEIGHT, "render widget", nullptr, nullptr);
    
    glfwSetWindowUserPointer(mWindow, this);
    glfwSetFramebufferSizeCallback(mWindow, FramebufferSizeCallback);
    glfwSetMouseButtonCallback(mWindow, MouseButtonCallback);
    glfwSetCursorPosCallback(mWindow, CursorPosCallback);
    glfwSetKeyCallback(mWindow, KeyCallback);

    glfwSetWindowSizeLimits(mWindow, 200, 200, GLFW_DONT_CARE, GLFW_DONT_CARE);
}

WindowTemplate::~WindowTemplate() {
    // 销毁glfw窗口
    glfwDestroyWindow(mWindow);
    glfwTerminate();
}

void WindowTemplate::Exec() {
    Initialize();
    while (!glfwWindowShouldClose(mWindow)) {
        glfwPollEvents();
        Update();
    }
    CleanUp();
}

VkSurfaceKHR WindowTemplate::CreateSurface(VkInstance instance) {
    VkSurfaceKHR surface;
    if (glfwCreateWindowSurface(instance, mWindow, nullptr, &surface) != VK_SUCCESS) {
        throw std::runtime_error("failed to create window surface!");
    }
    return surface;
}

std::vector<const char*> WindowTemplate::QueryWindowRequiredExtensions() {
    // 获取glfw的拓展
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
    return extensions;
}

VkExtent2D WindowTemplate::GetWindowExtent() {
    int width, height;
    glfwGetFramebufferSize(mWindow, &width, &height);

    VkExtent2D windowExtent = {
        static_cast<uint32_t>(width),
        static_cast<uint32_t>(height)
    };

    return windowExtent;
}

}