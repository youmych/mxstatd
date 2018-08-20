#pragma once
#include <epoll_actor.h>

namespace mxstatd
{

class ActorTcpReader : public linux::io::EpollActor
{
public:
    explicit ActorTcpReader(linux::io::EpollService& service, int sockfg);

    void ReadyRead() override;
    void ReadyWrite() override;
};

} // mxstatd
