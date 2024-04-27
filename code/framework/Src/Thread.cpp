#include "Thread.h"

namespace framework {
Thread::Thread() {
}

Thread::~Thread() {

}

void Thread::Start() {
    if (mThread == nullptr) {
        mThread = std::make_unique<std::thread>(&Thread::ThreadFunction, this);
    }
    std::unique_lock<std::mutex> lock(mThreadActiveMutex);
    mThreadActiveFlag = true;
    mThreadActiveCondition.notify_one();
}

void Thread::Stop() {
    if (mThread == nullptr) {
        mThread = std::make_unique<std::thread>(&Thread::ThreadFunction, this);
    }
    std::unique_lock<std::mutex> lock(mThreadActiveMutex);
    mThreadActiveFlag = false;
    mThreadActiveCondition.notify_one();
}

void Thread::Destroy() {
    if (mThread == nullptr) {
        return;
    }
    {
        std::unique_lock<std::mutex> lock(mThreadActiveMutex);
        mIsDestroying = true;
        mThreadActiveCondition.notify_one();
    }
    mThread->join();
    mThread = nullptr;
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

}   // namespace framework