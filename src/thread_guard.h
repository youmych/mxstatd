#pragma once

#include <thread>

/**
 * RAII idiom for exception-safe join threads.
 * Originally written by E.Whillams
 */
class ThreadGuard
{
    std::thread& m_Thread;
public:
    explicit ThreadGuard(std::thread& thread) : m_Thread(thread) {}

    ~ThreadGuard() {
        if( m_Thread.joinable() ) {
            m_Thread.join();
        }
    }

    ThreadGuard(ThreadGuard const&) = delete;
    ThreadGuard& operator=(ThreadGuard const&) = delete;
};
