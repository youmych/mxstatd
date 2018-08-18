#pragma once

#include <mutex>
#include <queue>
#include <condition_variable>

template <typename T>
class threadsafe_queue
{
    mutable std::mutex m_Mutex;
    std::queue<T> m_DataQueue;
    std::condition_variable m_DataCond;

public:
    threadsafe_queue() {}

    void push(T value) {
        std::lock_guard<std::mutex> lock(m_Mutex);
        m_DataQueue.push(std::move(value));
        m_DataCond.notify_one();
    }

    void wait_and_pop(T& value) {
        std::unique_lock<std::mutex> lock(m_Mutex);
        m_DataCond.wait(lock, [this](){ return !m_DataQueue.empty(); });
        value = std::move(m_DataQueue.front());
        m_DataQueue.pop();
    }

    std::shared_ptr<T> wait_and_pop() {
        std::unique_lock<std::mutex> lock(m_Mutex);
        m_DataCond.wait(lock, [this](){ return !m_DataQueue.empty(); });
        std::shared_ptr<T> res(
            std::make_shared<T>(std::move(m_DataQueue.front()))
        );
        m_DataQueue.pop();
        return res;
    }

    bool try_pop(T& value) {
        std::lock_guard<std::mutex> lock(m_Mutex);
        if( m_DataQueue.empty() ) {
            return false;
        }
        value = std::move(m_DataQueue.front());
        m_DataQueue.pop();
        return true;
    }

    std::shared_ptr<T> try_pop() {
        std::lock_guard<std::mutex> lock(m_Mutex);
        if( m_DataQueue.empty() )
            return std::shared_ptr<T>();
        std::shared_ptr<T> res(
            std::make_shared<T>(std::move(m_DataQueue.front()))
        );
        m_DataQueue.pop();
        return res;
    }

    bool empty() const {
        std::lock_guard<std::mutex> lock(m_Mutex);
        return m_DataQueue.empty();
    }
};
