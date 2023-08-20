#ifndef __WINDOW_IMPL_H__
#define __WINDOW_IMPL_H__

#include "WindowTemplate.h"
#include "RenderBase.h"

namespace window {
    class WindowImpl : public WindowTemplate
    {
    public:
        WindowImpl();
        ~WindowImpl();

    private:
        virtual void Initialize() override;
        virtual void Update() override;
        virtual void CleanUp() override;

    private:
        render::RenderBase* mRenderer = nullptr;

    };
}



#endif // !__WINDOW_IMPL_H__
