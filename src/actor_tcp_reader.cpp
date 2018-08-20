#include <actor_tcp_reader.h>
#include <epoll_service.h>
#include <net_utils.h>
#include <app_exceptions.h>

#include <iostream>
#include <iterator>

namespace mxstatd
{

ActorTcpReader::ActorTcpReader(linux::io::EpollService& service, int sockfg)
    : linux::io::EpollActor(service, sockfg, true)
{
}

void ActorTcpReader::ReadyRead()
{
    ssize_t rc = 0;
    char buf[100];
    // мы выставляли флаг EPOLLET, значит очередное событие вернется только
    // когда в сокет придут *новые* данные. Поэтому вычитываем всё.
    for(;;) {
        // rc = recv(NativeHandler(), get_buffer(mReadRequest), get_size(mReadRequest), 0);
        rc = recv(NativeHandler(), buf, 100, 0);
        if( rc < 0 ) {
            // Сокет асинхронный. Эти коды означают что вычитали все
            // данные из буфера
            if((errno == EAGAIN) || (errno == EWOULDBLOCK))
                return;
            throw mxstatd::system_error(errno, "ActorTcpReader.ReadyRead.recv");
        }
        if( rc == 0 ) { // eof
            throw std::system_error(std::make_error_code(std::io_errc::stream), "eof");
            // mReadRequest = ReadRequest();
            break;
        }

        std::copy(buf, buf+rc, std::ostream_iterator<char>(std::cout));
    }
}

void ActorTcpReader::ReadyWrite()
{

}

} // mxstatd

