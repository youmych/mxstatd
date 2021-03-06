#pragma once
#include <unordered_map>
#include <memory>
#include <mutex>

struct epoll_event;

namespace linux {
namespace io {

class EpollActorProxy;
class EpollActor;

/**
 * Сервис, обслуживающий множество файловых дескрипторов.
 * Некоторый аналог boost::asio::io_service
 */
class EpollService
{
    using actor_proxy_ptr_t = std::shared_ptr<EpollActorProxy>;
    using actor_ptr_t = std::shared_ptr<EpollActor>;

    static constexpr size_t MAX_EVENTS = 100;

    /// Дескриптор для epoll
    int m_PollFd = -1;
    /// для пайпа остановки
    int m_StopEventFds[2] = { -1, -1 };
    /// все активные агенты
    std::unordered_map<EpollActorProxy*, actor_proxy_ptr_t> m_Actors;
    /// мьютекс для защиты хранилища агентов
    std::mutex m_MapMutex;

public:
    explicit EpollService();
    ~EpollService();

    // не копируемый класс (хранит ресурсы)
    EpollService(EpollService const&) = delete;
    EpollService& operator=(EpollService const&) = delete;

    void Run();
    void Stop();
    void Unregister(actor_proxy_ptr_t actor);
    void Register(actor_ptr_t actor);

    /// Метод для создания агентов
    template<class T, class... Args>
    void CreateActor(Args&&... args) {
        Register( std::make_shared<T>(*this, std::forward<Args>(args)...) );
    }

private:
    void Close();
    void HandleEvent(struct epoll_event& ev);
    void StopAll();

    int StopEventReadFd() const { return m_StopEventFds[0]; }
    int StopEventWriteFd() const { return m_StopEventFds[1]; }
};

} // namespace io
} // namespace linux
