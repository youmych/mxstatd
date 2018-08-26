#include <net_utils.h>

#include <iostream>
#include <cstring>

#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <app_exceptions.h>

static constexpr int NLISTEN = 100;

//------------------------------------------------------------------------------
bool isvalidsock(int sockfd)
{
    return (sockfd != (-1));
}

//------------------------------------------------------------------------------
SocketCloser::~SocketCloser() {
    if( is_Own && isvalidsock(m_Sockfd) ) {
        close(m_Sockfd);
        m_Sockfd = -1;
    }
}

//------------------------------------------------------------------------------
int set_nonblocking(int sockfd)
{
    int nonblocking = 1;

    int flags = fcntl (sockfd, F_GETFL, 0);
    if (flags == -1) {
        throw mxstatd::system_error(errno, "fcntl(F_GETFL)");
    }
    flags |= O_NONBLOCK;
    if( 0 != fcntl(sockfd, F_SETFL, flags, nonblocking) )
        throw mxstatd::system_error(errno, "fcntl(F_SETFL)");

    return sockfd;
}
//------------------------------------------------------------------------------
void set_address(const char* hostName, const char* serviceName,
    struct sockaddr_in* sap, const char* protocol)
{
    struct servent *sp;
    char *endp;

    memset(sap, 0, sizeof(*sap));
    sap->sin_family = AF_INET;
    if( nullptr != hostName ) {
        if( !inet_aton(hostName, &sap->sin_addr) ) {
            struct hostent *hp = gethostbyname(hostName);
            if( nullptr == hp ) {
                std::cout << "Unknown host: " << hostName << std::endl;
                throw mxstatd::host_error(h_errno, hostName);
            }
            sap->sin_addr = *(struct in_addr*)hp->h_addr;
        }
    }
    else {
        sap->sin_addr.s_addr = htonl( INADDR_ANY );
    }

    short port = strtol(serviceName, &endp, 0);
    if( *endp == '\0' )
        sap->sin_port = htons(port);
    else {
        sp = getservbyname(serviceName, protocol);
        if( nullptr == sp ) {
            std::cout << "Unknown service: " << serviceName << std::endl;
            throw mxstatd::unknown_service(serviceName);
        }
    }
}

//------------------------------------------------------------------------------
int tcp_server_socket(struct sockaddr* sockaddr)
{
    int on = 1;

    int af = sockaddr->sa_family;
    if( af != AF_INET && af != AF_INET6 ) {
        errno = EINVAL;
        throw mxstatd::system_error(errno, "tcp_server_socket");
    }
    socklen_t sockAddrLen = (af == AF_INET ? sizeof(struct sockaddr_in) : sizeof(struct sockaddr_in6));

    int s = socket( af, SOCK_STREAM, 0 );
    if( !isvalidsock(s) ) {
        throw mxstatd::system_error(errno, "tcp_server.socket");
    }
    SocketCloser sc(s);

    if( 0 != setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) ) {
        throw mxstatd::system_error(errno, "tcp_server.setsockopt");
    }

    if( 0 != bind(s, sockaddr, sockAddrLen) ) {
        throw mxstatd::system_error(errno, "tcp_server.bind");
    }

    sc.UnOwn();
    return s;
}

//------------------------------------------------------------------------------
int tcp4_server(const char* hostName, const char* serviceName)
{
    struct sockaddr_in local;

    set_address(hostName, serviceName, &local, "tcp");
    int s = tcp_server_socket((struct sockaddr*)&local);
    set_nonblocking(s);
    if( 0 != listen(s, NLISTEN) ) {
        throw mxstatd::system_error(errno, "tcp4_server.listen");
    }
    return s;
}

//------------------------------------------------------------------------------
int tcp4_server(const char* hostName, int port)
{
    char portName[10];
    snprintf(portName, sizeof portName, "%d", port);
    return tcp4_server(hostName, portName);
}

//------------------------------------------------------------------------------
int udp4_server(const char* hostName, const char* serviceName)
{
    struct sockaddr_in local;

    set_address(hostName, serviceName, &local, "udp");
    int s = socket(AF_INET, SOCK_DGRAM, 0);
    if( !isvalidsock(s) )
        throw mxstatd::system_error(errno, "udp4_server.socket");
    SocketCloser sc(s);

    if( 0 != bind(s, (struct sockaddr*)&local, sizeof(local)) )
        throw mxstatd::system_error(errno, "udp4_server.bind");

    sc.UnOwn();
    return s;
}
//------------------------------------------------------------------------------
int udp4_server(const char* hostName, int port)
{
    char portName[10];
    snprintf(portName, sizeof portName, "%d", port);
    return udp4_server(hostName, portName);
}

//------------------------------------------------------------------------------
namespace linux {
namespace netdb {

//------------------------------------------------------------------------------
const char* HostentErrorCategory::name() const noexcept
{
  return "netdb";
}

//------------------------------------------------------------------------------
std::string HostentErrorCategory::message(int ev) const
{
  switch (static_cast<HostentError>(ev))
  {
  case HostentError::host_not_found:
  case HostentError::no_data:
  case HostentError::no_recovery:
  case HostentError::try_again:
    return hstrerror(ev);

  default:
    return "(unrecognized error)";
  }
}

//-----------------------------------------------------------------------------
const char* ServentErrorCategory::name() const noexcept
{
  return "netdb";
}

//------------------------------------------------------------------------------
std::string ServentErrorCategory::message(int ev) const
{
  switch (static_cast<ServentError>(ev))
  {
  case ServentError::service_not_found:
    return "service not found";
  default:
    return "(unrecognized error)";
  }
}

//------------------------------------------------------------------------------
const HostentErrorCategory hostent_error_category {};
const ServentErrorCategory servent_error_category {};

} // namespace netdb
} // namespace linux

std::error_code make_error_code(linux::netdb::HostentError e)
{
  return {static_cast<int>(e), linux::netdb::hostent_error_category};
}

std::error_code make_error_code(linux::netdb::ServentError e)
{
  return {static_cast<int>(e), linux::netdb::servent_error_category};
}
