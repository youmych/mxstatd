#include <epoll_service.h>
#include <epoll_actor_proxy.h>
#include <epoll_actor.h>
#include <app_exceptions.h>

#include <iterator>
#include <iostream>
#include <algorithm>
#include <cstring>
#include <cassert>

#include <sys/epoll.h>
#include <unistd.h>
#include <fcntl.h>

namespace linux {
namespace io {

//-----------------------------------------------------------------------------
EpollService::EpollService()
{
    m_PollFd = epoll_create1(0);
    if( m_PollFd < 0 )
        throw mxstatd::system_error(errno, "epoll_create1()");
    if( pipe2(m_StopEventFds, O_DIRECT | O_NONBLOCK) < 0 )
        throw mxstatd::system_error(errno, "pipe2()");

    auto stopProxy = std::make_shared<EpollActorProxy>(*this, StopEventReadFd());
    struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.ptr = stopProxy.get();
    if (epoll_ctl(m_PollFd, EPOLL_CTL_ADD, StopEventReadFd(), &ev) == -1) {
        throw mxstatd::system_error(errno, "epoll_ctl(add_stop_handler)");
    }

    m_Actors.emplace(stopProxy.get(), stopProxy);
}

//-----------------------------------------------------------------------------
EpollService::~EpollService()
{
    StopAll();
    Close();
}

//-----------------------------------------------------------------------------
void EpollService::Run()
{
    struct epoll_event events[MAX_EVENTS];
    for(;;) {
        auto nfds = epoll_wait(m_PollFd, events, std::size(events), -1);
        if (nfds == -1) {
            if(errno == EINTR) {
                continue;
            }
            throw mxstatd::system_error(errno, "epoll_wait()");
        }
        std::for_each(events, events + nfds, [this](auto ev){
            HandleEvent(ev);
        });
    }
}

//-----------------------------------------------------------------------------
void EpollService::Stop()
{
    size_t n = 1;
    if( write(StopEventWriteFd(), &n, sizeof n) < 0 ) {
        // do nothing
    }
}

//-----------------------------------------------------------------------------
void EpollService::Close()
{
    if( m_PollFd >= 0 ) {
        close(m_PollFd);
        m_PollFd = -1;
    }
    if( m_StopEventFds[0] >= 0 ) {
        close(m_StopEventFds[0]);
        m_StopEventFds[0] = -1;
    }
    if( m_StopEventFds[1] >= 0 ) {
        close(m_StopEventFds[1]);
        m_StopEventFds[1] = -1;
    }
}

//-----------------------------------------------------------------------------
void EpollService::HandleEvent(struct epoll_event& ev)
{
    auto found = m_Actors.find(static_cast<EpollActorProxy*>(ev.data.ptr));
    assert(found != m_Actors.end());
    if( found == m_Actors.end() ) {
        return;
    }

    auto pactor = found->second;
    if( pactor->NativeHandler() == StopEventReadFd() )
        throw mxstatd::epoll_service_terminate();

    //ev.events
    try {
        if(ev.events & (EPOLLOUT | EPOLLERR | EPOLLHUP) ) {
            pactor->ReadyWrite(ev.events);
        }

        if(ev.events & (EPOLLIN | EPOLLERR | EPOLLHUP | EPOLLRDHUP)) {
            pactor->ReadyRead(ev.events);
        }
    }
    catch(mxstatd::epoll_service_terminate& ) {
        throw;
    }
    catch(std::exception& e) {
        std::cout << e.what() << std::endl;
        pactor->Stop();
    }
}

//-----------------------------------------------------------------------------
void EpollService::Unregister(actor_proxy_ptr_t actor)
{
    if( !actor )
        return;

    if( 0 != epoll_ctl(m_PollFd, EPOLL_CTL_DEL, actor->NativeHandler(), nullptr) ) {
        std::cerr << "Warning: can't unregister epoll handler "
            << actor->NativeHandler() << ": " << strerror(errno) << std::endl;
    }
    m_Actors.erase( actor.get() );
}

//-----------------------------------------------------------------------------
void EpollService::Register(actor_ptr_t actor)
{
    if( !actor )
        return;

    int fd = actor->NativeHandler();
    auto proxy = std::make_shared<EpollActorProxy>(*this, fd, std::move(actor));
    struct epoll_event ev;
    ev.events = EPOLLIN;
    if( actor->EventTriggered() )
        ev.events |= (EPOLLOUT | EPOLLET);
    ev.data.ptr = proxy.get();
    if (epoll_ctl(m_PollFd, EPOLL_CTL_ADD, fd, &ev) == -1) {
        throw mxstatd::system_error(errno, "epoll_ctl(add actor)");
    }
    m_Actors.emplace(proxy.get(), std::move(proxy));
}

//-----------------------------------------------------------------------------
void EpollService::StopAll()
{
    for( auto [unused, actor]: m_Actors ) {
        try {
            actor->Stop();
            (void)(unused);
        }
        catch(std::exception&) {}
    }
}

} // namespace io
} // namespace linux
