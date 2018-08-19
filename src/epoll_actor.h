#pragma once

namespace linux {
namespace io {

class EpollActor
{
    bool is_EventTriggered = false;
public:
    EpollActor(bool isEventTriggered = false)
        : is_EventTriggered(isEventTriggered)
    {}

    virtual ~EpollActor() {}

    bool EventTriggered() const { return is_EventTriggered; }

    virtual void ReadyRead() = 0;
    virtual void ReadyWrite() = 0;
};

} // namespace io
} // namespace linux
