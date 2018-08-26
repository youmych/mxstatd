#pragma once
#include <epoll_actor.h>
#include <string>
#include <buffer_cutter.h>
#include <event_queue.h>

namespace mxstatd {

class ActorNamedPipeReader : public linux::io::EpollActor
{
    std::string m_PipeName;
    BufferCutter m_Cutter;
    std::shared_ptr<EventQueue::Collector> m_Collector;
    size_t total_ = 0;
public:
    explicit ActorNamedPipeReader(linux::io::EpollService& service,
        const std::string& pipeName);

    void ReadyRead() override;
    void ReadyWrite() override;
};

} // namespace mxstatd
