#include "WindowImpl.h"
#include "Utils.h"

namespace window {
    WindowImpl::WindowImpl(bool resizable) : WindowTemplate(resizable)
    {
        mRenderer = new render::RenderBase(*this);
    }

    WindowImpl::~WindowImpl() {
        delete mRenderer;
    }

    void WindowImpl::Initialize() {
        mRenderer->Init(setting::enableValidationLayer);
    }

    void WindowImpl::Update() {
        mRenderer->Update();
    }

    void WindowImpl::CleanUp() {
        mRenderer->CleanUp();
    }

    void WindowImpl::OnFramebufferResize(GLFWwindow* window, int width, int height) {
        mRenderer->SetFramebufferResized();
    }


}
