#pragma once

#include <epoll_actor.h>
#include <netinet/in.h>

namespace mxstatd {

class ActorTcpListener : public linux::io::EpollActor
{
    struct sockaddr_in m_LocalAddr;
public:
    explicit ActorTcpListener(linux::io::EpollService& service, int port);
    ~ActorTcpListener();

    void ReadyRead() override;
    void ReadyWrite() override;
};

} // namespace mxstatd
