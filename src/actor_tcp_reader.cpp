#include <actor_tcp_reader.h>
#include <epoll_service.h>
#include <net_utils.h>
#include <app_exceptions.h>

#include <iostream>
#include <iterator>
#include <algorithm>
#include <numeric>

namespace mxstatd
{

ActorTcpReader::ActorTcpReader(linux::io::EpollService& service, int sockfg)
    : linux::io::EpollActor(service, sockfg, true)
{
}

void ActorTcpReader::ReadyRead()
{
    ssize_t rc = 0;
    char buf[40*1024];
    // мы выставляли флаг EPOLLET, значит очередное событие вернется только
    // когда в сокет придут *новые* данные. Поэтому вычитываем всё.
    for(;;) {
        // rc = recv(NativeHandler(), get_buffer(mReadRequest), get_size(mReadRequest), 0);
        rc = recv(NativeHandler(), buf, sizeof(buf), 0);
        if( rc < 0 ) {
            // Сокет асинхронный. Эти коды означают что вычитали все
            // данные из буфера
            if((errno == EAGAIN) || (errno == EWOULDBLOCK))
                return;
            throw mxstatd::system_error(errno, "ActorTcpReader.ReadyRead.recv");
        }
        if( rc == 0 ) { // eof
            // аккуратное разымкание соединения
            shutdown(NativeHandler(), SHUT_RDWR);
            throw std::system_error(std::make_error_code(std::io_errc::stream), "eof");
        }

        m_Cutter(std::string_view(buf, rc), [this](auto){
            ++m_;
        });
    }
}

void ActorTcpReader::ReadyWrite()
{

}

} // mxstatd

