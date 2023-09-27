#include "WindowImpl.h"
#include "Utils.h"

namespace window {
    WindowImpl::WindowImpl(bool resizable) : WindowTemplate(resizable)
    {
        mRenderThread = new render::DrawTriangleThread(*this);
        //mRenderThread = new render::DrawTextureThread(*this);
    }

    WindowImpl::~WindowImpl() {
        mRenderThread->Destroy();
        delete mRenderThread;
    }

    void WindowImpl::Initialize() {
        mRenderThread->Start();
    }

    void WindowImpl::Update() {

    }

    void WindowImpl::CleanUp() {
        mRenderThread->Stop();
    }

    void WindowImpl::OnFramebufferResized(int width, int height) {
        static_cast<render::DrawTriangleThread*>(mRenderThread)->SetFramebufferResized();
        //static_cast<render::DrawTextureThread*>(mRenderThread)->SetFramebufferResized();

        if (width == 0 || height == 0) {
            if (mIsMinimized == false) {
                mRenderThread->Stop();    // 最小化时停止渲染线程
            }
            mIsMinimized = true;
        }
        else {
            if (mIsMinimized == true) {
                mRenderThread->Start();   // 恢复时打开渲染线程
            }
            mIsMinimized = false;
        }
    }


}
