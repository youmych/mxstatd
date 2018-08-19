#pragma once

bool isvalidsock(int sockfd);

void set_nonblocking(int fd);

void set_address(const char* hostName, const char* serviceName,
    struct sockaddr_in* sa, const char* protocol);

int tcp_server_socket(struct sockaddr* sockaddr, int maxListeners);

int tcp4_server(const char* hostName, const char* serviceName);

int udp4_server(const char* hostName, const char* serviceName);
