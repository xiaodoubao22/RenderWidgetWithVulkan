#include "WindowImpl.h"
#include "Utils.h"

namespace window {
    WindowImpl::WindowImpl(bool resizable) : WindowTemplate(resizable)
    {
        mDrawTriangleThread = new render::DrawTriangleThread(*this);
    }

    WindowImpl::~WindowImpl() {
        mDrawTriangleThread->Destroy();
        delete mDrawTriangleThread;
    }

    void WindowImpl::Initialize() {
        mDrawTriangleThread->Start();
    }

    void WindowImpl::Update() {
    }

    void WindowImpl::CleanUp() {
        mDrawTriangleThread->Stop();
    }

    void WindowImpl::OnFramebufferResized(GLFWwindow* window, int width, int height) {
        mDrawTriangleThread->SetFramebufferResized();

    }


}
