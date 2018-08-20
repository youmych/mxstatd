#include <actor_tcp_listener.h>
#include <net_utils.h>
#include <epoll_service.h>
#include <actor_tcp_reader.h>

#include <memory>

#include <sys/types.h>
#include <sys/socket.h>


namespace mxstatd {

ActorTcpListener::ActorTcpListener(linux::io::EpollService& service, int port)
    : linux::io::EpollActor( service, tcp4_server(nullptr, port) )
{

}

void ActorTcpListener::ReadyRead()
{
    struct sockaddr_in peerAddr;
    socklen_t addrLen = sizeof(peerAddr);
    int clientfd = accept(NativeHandler(), (struct sockaddr*)&peerAddr, &addrLen);

    SocketCloser sc(clientfd);
    set_nonblocking(clientfd);

    Service().Register( clientfd, std::make_shared<ActorTcpReader>(Service(), clientfd) );
    sc.UnOwn();
}

void ActorTcpListener::ReadyWrite()
{
    // do nothing
}

} // namespace mxstatd
