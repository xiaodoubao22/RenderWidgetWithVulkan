#include "Thread.h"

namespace common {
    Thread::Thread() {
        mThread = std::thread(&Thread::ThreadFunction, this);
    }

    Thread::~Thread() {

    }

    void Thread::Start() {
        std::unique_lock<std::mutex> lock(mThreadActiveMutex);
        mThreadActiveFlag = true;
        mThreadActiveCondition.notify_one();
    }

    void Thread::Stop() {
        std::unique_lock<std::mutex> lock(mThreadActiveMutex);
        mThreadActiveFlag = false;
        mThreadActiveCondition.notify_one();
    }

    void Thread::Destroy() {
        {
            std::unique_lock<std::mutex> lock(mThreadActiveMutex);
            mIsDestroying = true;
            mThreadActiveCondition.notify_one();
        }
        mThread.join();
    }

    void Thread::PushData(std::string& lable, void* data)
    {
        return;
    }

    void Thread::SetFbResized()
    {
        mFramebufferResized.store(true);
    }

    void Thread::ResetFbResized()
    {
        mFramebufferResized.store(false);
    }

    bool Thread::IsFbResized()
    {
        return mFramebufferResized.load();
    }

    void Thread::ThreadFunction() {
        OnThreadInit();

        while (true) {
            {
                std::unique_lock<std::mutex> lock(mThreadActiveMutex);
                mThreadActiveCondition.wait(lock, [this]() {return mThreadActiveFlag || mIsDestroying; });
                if (mIsDestroying) {
                    break;
                }
            }

            OnThreadLoop();

            {
                std::unique_lock<std::mutex> lock(mThreadActiveMutex);
                mThreadActiveCondition.notify_one();
            }
        }

        OnThreadDestroy();
    }

}