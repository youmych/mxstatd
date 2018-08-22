#pragma once
#include <epoll_actor.h>
#include <string>
#include <line_cutter.h>

namespace mxstatd {

class ActorNamedPipeReader : public linux::io::EpollActor
{
    std::string m_PipeName;
    LineCutter m_Cutter;
    size_t n_ = 0;
    size_t m_ = 0;
    size_t total_ = 0;
public:
    explicit ActorNamedPipeReader(linux::io::EpollService& service,
        const std::string& pipeName);

    void ReadyRead() override;
    void ReadyWrite() override;
};

} // namespace mxstatd
