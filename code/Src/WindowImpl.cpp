#include "WindowImpl.h"
#include "Utils.h"

namespace window {
    WindowImpl::WindowImpl() {
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
}
