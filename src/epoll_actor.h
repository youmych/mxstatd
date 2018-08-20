#pragma once

namespace linux {
namespace io {

class EpollService;
class EpollActor
{
    EpollService& m_Service;
    bool is_EventTriggered = false;
    int m_Fd = -1;
public:
    EpollActor(EpollService& service, int fd, bool isEventTriggered = false)
        : m_Service(service)
        , is_EventTriggered(isEventTriggered)
        , m_Fd(fd)
    {}

    virtual ~EpollActor() {}

    bool EventTriggered() const { return is_EventTriggered; }

    virtual void ReadyRead() = 0;
    virtual void ReadyWrite() = 0;

    int NativeHandler() const { return m_Fd; }

    EpollService& Service() { return m_Service; }
};

} // namespace io
} // namespace linux
