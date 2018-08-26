#include <actor_udp_server.h>
#include <epoll_service.h>
#include <net_utils.h>
#include <app_exceptions.h>
#include <statistics.h>
#include <stat_map.h>
#include <data_log_event.h>

#include <iostream>
#include <sstream>
#include <cstring>

namespace mxstatd
{

//-----------------------------------------------------------------------------
ActorUdpServer::ActorUdpServer(linux::io::EpollService& service, int port)
    : linux::io::EpollActor(service, set_nonblocking(udp4_server(nullptr, port)))
    , m_Port(port)
{
    std::cout << "UDP server ready at port " << port << std::endl;
}
//-----------------------------------------------------------------------------
ActorUdpServer::~ActorUdpServer()
{
    std::cout << "UDP server at port " << m_Port << " done" << std::endl;
}
//-----------------------------------------------------------------------------
void ActorUdpServer::ReadyRead()
{
    struct sockaddr_in peer;
    socklen_t peerlen;
    char buf[64*1024];

    for(;;) {
        peerlen = sizeof(peer);
        auto rc = recvfrom(NativeHandler(), buf, sizeof(buf), 0,
            (struct sockaddr*)&peer, &peerlen);
        if( rc < 0 ) {
            if( errno == EAGAIN || errno == EWOULDBLOCK )
                break;
            mxstatd::system_error(errno, "UdpServer.recvfrom");
        }

        // для простоты будем обрабатывать только начало буфера.
        auto stat = STAT().GetStatistics( Data::Log::Event::from_string(buf, rc) );

        std::stringstream msgbuf;
        if( stat ) {
            stat->PrintGeneric(msgbuf);
        }
        else {
            msgbuf << "UNKNOWN EVENT TYPE";
        }

        auto msg = msgbuf.str();
        rc = sendto(NativeHandler(), msg.c_str(), msg.length(), 0, (struct sockaddr*)&peer, peerlen);
        if( rc < 0 ) {
            mxstatd::system_error(errno, "UdpServer.sendto");
        }
    }
}
//-----------------------------------------------------------------------------
void ActorUdpServer::ReadyWrite()
{

}

} // mxstatd
