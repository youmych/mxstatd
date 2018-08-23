#pragma once

#include <deque>
#include <chrono>

#include <threadsafe-queue.h>
#include <data_log_event.h>

namespace mxstatd
{

class EventQueue
{
    explicit EventQueue();
    ~EventQueue();

    // чтобы статистика могла забираться из очереди
    friend class Statistics;

public:
    using event_t = Data::Log::Event;
    using queue_item_t = std::deque<event_t>;
    using queue_t = threadsafe_queue<queue_item_t>;

    /// Адаптор к очереди, позволяющий добавлять события поштучно
    class Collector {
        queue_t& m_Queue;
        queue_item_t m_Samples;
        std::chrono::high_resolution_clock::time_point m_FirstAppendTime;

        static constexpr size_t MAX_SAMPLES = 100;
        static constexpr auto MAX_COLLECT_TIME = std::chrono::seconds(1);
    public:
        explicit Collector(queue_t& q);
        ~Collector();

        Collector(const Collector&) = delete;
        Collector& operator=(const Collector&) = delete;

        void AppendEvent(event_t event);
        void Flush();

        static auto Now() { return std::chrono::high_resolution_clock::now(); }
    };

    static EventQueue& Instance() {
        static EventQueue q;
        return q;
    }

    EventQueue(EventQueue &&) = delete;
    EventQueue(const EventQueue &) = delete;
    EventQueue &operator=(EventQueue &&) = delete;
    EventQueue &operator=(const EventQueue &) = delete;

    static std::shared_ptr<Collector> MakeCollector() {
        return std::make_shared<Collector>( Instance().m_EventQueue );
    }

private:
    queue_t m_EventQueue;
};

} // mxstatd
