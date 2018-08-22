#pragma once

#include <netdb.h>
#include <system_error>

namespace linux {
namespace netdb {

enum class HostentError {
    host_not_found = HOST_NOT_FOUND,
    no_data = NO_DATA,
    no_recovery = NO_RECOVERY,
    try_again = TRY_AGAIN
};

enum class ServentError {
    service_not_found = 1
};

struct HostentErrorCategory : std::error_category
{
  const char* name() const noexcept override;
  std::string message(int ev) const override;
};

struct ServentErrorCategory : std::error_category
{
  const char* name() const noexcept override;
  std::string message(int ev) const override;
};

} // namespace netdb
} // namespace linux

namespace std
{
  template <>
    struct is_error_code_enum<linux::netdb::HostentError> : true_type {};

  template <>
    struct is_error_code_enum<linux::netdb::ServentError> : true_type {};
} // nmaespace std

std::error_code make_error_code(linux::netdb::HostentError);
std::error_code make_error_code(linux::netdb::ServentError);

bool isvalidsock(int sockfd);

int set_nonblocking(int fd);

void set_address(const char* hostName, const char* serviceName,
    struct sockaddr_in* sa, const char* protocol);

///
int tcp_server_socket(struct sockaddr* sockaddr);

///
int tcp4_server(const char* hostName, const char* serviceName);
int tcp4_server(const char* hostName, int port);

///
int udp4_server(const char* hostName, const char* serviceName);
int udp4_server(const char* hostName, int port);

/**
 *
 */
class SocketCloser {
    int& m_Sockfd;
    bool is_Own = true;
public:
    SocketCloser(int& sockfd) : m_Sockfd(sockfd) {}
    ~SocketCloser();

    void UnOwn() { is_Own = false; }
};
