#include <actor_named_pipe_reader.h>
#include <app_exceptions.h>
#include <epoll_service.h>

#include <iostream>
#include <numeric>
#include <algorithm>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

namespace mxstatd
{

//-----------------------------------------------------------------------------
static int
create_and_open_named_pipe(const std::string& pipeName)
{
    int rc = mkfifo(pipeName.c_str(), 00666); // !Octal number!
    if( (rc < 0) && (EEXIST != errno) ) {
        throw mxstatd::system_error(errno, "create_and_open_named_pipe.mkfifo");
    }

    rc = open(pipeName.c_str(), O_RDONLY | O_NONBLOCK);
    if( rc < 0 )
        throw mxstatd::system_error(errno, "create_and_open_named_pipe.open");

    return rc;
}
//-----------------------------------------------------------------------------
ActorNamedPipeReader::ActorNamedPipeReader(linux::io::EpollService& service,
    const std::string& pipeName)
    : linux::io::EpollActor(service, create_and_open_named_pipe(pipeName), true)
    , m_PipeName(pipeName)
{

}
//-----------------------------------------------------------------------------
void ActorNamedPipeReader::ReadyRead()
{
    ssize_t rc = 0;
    char buf[40*1024]; // 40 байт средняя длина строки. Уменьшаем фрагментирование.
    // мы выставляли флаг EPOLLET, значит очередное событие вернется только
    // когда в сокет придут *новые* данные. Поэтому вычитываем всё.
    for(;;) {
        rc = read(NativeHandler(), buf, sizeof(buf));
        if( rc < 0 ) {
            // Сокет асинхронный. Эти коды означают что вычитали все
            // данные из буфера
            if((errno == EAGAIN) || (errno == EWOULDBLOCK))
                return;
            throw mxstatd::system_error(errno, "ActorTcpReader.ReadyRead.recv");
        }
        if( rc == 0 ) { // eof
            std::cout << "EOF. Readed " << m_Cutter.LineCount()
                << " lines, "
                << m_Cutter.Partials() << " partials. "
                << " Total bytes: " << total_  << std::endl;

            Service().CreateActor<ActorNamedPipeReader>(m_PipeName);

            throw std::system_error(std::make_error_code(std::io_errc::stream), "eof");
        }

        total_ += rc;
        n_ += std::count_if(buf, buf+rc, [](char c){ return c =='\n';} );
        m_Cutter(std::string_view(buf, rc), [this](auto){
            ++m_;
        });
    }
}
//-----------------------------------------------------------------------------
void ActorNamedPipeReader::ReadyWrite()
{
    // do nothing. We read only
}

} // mxstatd
