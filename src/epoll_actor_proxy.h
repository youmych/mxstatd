#pragma once
#include <memory>

namespace linux {
namespace io {

class EpollService;
class EpollActor;

class EpollActorProxy
    : public std::enable_shared_from_this<EpollActorProxy>
{
    using actor_ptr_t = std::shared_ptr<EpollActor>;

    EpollService& m_Service;
    int m_Fd = -1;
    actor_ptr_t m_Actor;

    friend class EpollService;
public:
    explicit EpollActorProxy(EpollService& service, int fd, actor_ptr_t actor = nullptr);
    ~EpollActorProxy();

    EpollActorProxy(const EpollActorProxy&) = delete;
    EpollActorProxy& operator=(const EpollActorProxy&) = delete;

    void Stop();
    int NativeHandler() const { return m_Fd; }
};

} // namespace io
} // namespace linux
