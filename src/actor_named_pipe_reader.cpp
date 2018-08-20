#include <actor_named_pipe_reader.h>
#include <app_exceptions.h>

#include <iostream>
#include <numeric>

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
    : linux::io::EpollActor(service, create_and_open_named_pipe(pipeName))
    , m_PipeName(pipeName)
{

}
//-----------------------------------------------------------------------------
void ActorNamedPipeReader::ReadyRead()
{
    ssize_t rc = 0;
    char buf[1024*64];
    // мы выставляли флаг EPOLLET, значит очередное событие вернется только
    // когда в сокет придут *новые* данные. Поэтому вычитываем всё.
    for(;;) {
        // rc = recv(NativeHandler(), get_buffer(mReadRequest), get_size(mReadRequest), 0);
        rc = read(NativeHandler(), buf, sizeof(buf));
        if( rc < 0 ) {
            // Сокет асинхронный. Эти коды означают что вычитали все
            // данные из буфера
            if((errno == EAGAIN) || (errno == EWOULDBLOCK))
                return;
            throw mxstatd::system_error(errno, "ActorTcpReader.ReadyRead.recv");
        }
        if( rc == 0 ) { // eof
            throw std::system_error(std::make_error_code(std::io_errc::stream), "eof");
        }

        // std::copy(buf, buf+rc, std::ostream_iterator<char>(std::cout));
        size_t summ = std::accumulate(buf, buf+rc, size_t(0));
        std::cout << "summ of " << rc << " items = " << summ << std::endl;
    }
}
//-----------------------------------------------------------------------------
void ActorNamedPipeReader::ReadyWrite()
{
    // do nothing. We read only
}

} // mxstatd
