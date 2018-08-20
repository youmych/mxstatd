#pragma once
#include <epoll_actor.h>
#include <string>

namespace mxstatd {

class ActorNamedPipeReader : public linux::io::EpollActor
{
    std::string m_PipeName;
public:
    explicit ActorNamedPipeReader(linux::io::EpollService& service,
        const std::string& pipeName);

    void ReadyRead() override;
    void ReadyWrite() override;
};

} // namespace mxstatd
