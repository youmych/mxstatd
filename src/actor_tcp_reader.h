#pragma once
#include <epoll_actor.h>
#include <buffer_cutter.h>
#include <event_queue.h>

namespace mxstatd
{

class ActorTcpReader : public linux::io::EpollActor
{
    BufferCutter m_Cutter;
    std::shared_ptr<EventQueue::Collector> m_Collector;
    size_t total_ = 0;
public:
    explicit ActorTcpReader(linux::io::EpollService& service, int sockfg);

    void ReadyRead() override;
    void ReadyWrite() override;
};

} // mxstatd
