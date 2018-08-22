#pragma once
#include <epoll_actor.h>

#include <line_cutter.h>

namespace mxstatd
{

class ActorTcpReader : public linux::io::EpollActor
{
    LineCutter m_Cutter;
    size_t n_ = 0;
    size_t m_ = 0;
    size_t total_ = 0;
public:
    explicit ActorTcpReader(linux::io::EpollService& service, int sockfg);

    void ReadyRead() override;
    void ReadyWrite() override;
};

} // mxstatd
