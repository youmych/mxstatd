#pragma once

#include <epoll_actor.h>

namespace mxstatd {

class ActorUdpServer : public linux::io::EpollActor
{
    int m_Port = -1;
public:
    ActorUdpServer(linux::io::EpollService& service, int port);
    virtual ~ActorUdpServer();

    void ReadyRead() override;
    void ReadyWrite() override;
};

} // namespace mxstatd
