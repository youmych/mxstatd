#include <epoll_actor_proxy.h>
#include <epoll_actor.h>
#include <epoll_service.h>

#include <sys/epoll.h>

namespace linux {
namespace io {

//-----------------------------------------------------------------------------
EpollActorProxy::EpollActorProxy(EpollService& service, int fd, actor_ptr_t actor)
    : m_Service(service)
    , m_Fd(fd)
    , m_Actor(actor)
{

}

//-----------------------------------------------------------------------------
EpollActorProxy::~EpollActorProxy()
{

}
//-----------------------------------------------------------------------------
void EpollActorProxy::ReadyRead(int /*events*/)
{
    if( m_Actor )
        m_Actor->ReadyRead();
}
//-----------------------------------------------------------------------------
void EpollActorProxy::ReadyWrite(int /*events*/)
{
    if( m_Actor )
        m_Actor->ReadyWrite();
}
//-----------------------------------------------------------------------------
void EpollActorProxy::Stop()
{
    m_Service.Unregister( shared_from_this() );
}

} // namespace io
} // namespace linux
