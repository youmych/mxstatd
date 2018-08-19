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
class SocketCloser {
    int& m_Sockfd;
    bool is_Own = true;
public:
    SocketCloser(int& sockfd) : m_Sockfd(sockfd) {}
    ~SocketCloser() {
        if( is_Own && isvalidsock(m_Sockfd) ) {
            close(m_Sockfd);
            m_Sockfd = -1;
        }
    }

    void UnOwn() { is_Own = false; }
};

//------------------------------------------------------------------------------
void set_nonblocking(int sockfd)
{
    int nonblocking = 1;

    int flags = fcntl (sockfd, F_GETFL, 0);
    if (flags == -1) {
        throw mxstatd::system_error(errno, "fcntl(F_GETFL)");
    }
    flags |= O_NONBLOCK;
    if( 0 != fcntl(sockfd, F_SETFL, flags, nonblocking) )
        throw mxstatd::system_error(errno, "fcntl(F_SETFL)");
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
                throw mxstatd::unknown_host();
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
            throw mxstatd::unknown_service();
        }
    }
}

//------------------------------------------------------------------------------
int tcp_server_socket(struct sockaddr* sockaddr, int nlisten)
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

    if( 0 != listen(s, nlisten) ) {
        throw mxstatd::system_error(errno, "tcp_server.listen");
    }

    sc.UnOwn();
    return s;
}

//------------------------------------------------------------------------------
int tcp4_server(const char* hostName, const char* serviceName)
{
    struct sockaddr_in local;

    set_address(hostName, serviceName, &local, "tcp");
    return tcp_server_socket((struct sockaddr*)&local, NLISTEN);
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
