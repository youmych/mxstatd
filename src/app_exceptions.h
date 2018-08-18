#pragma once

#include <exception> // exception
#include <stdexcept> // runtime_error
#include <system_error> // system_error, system_category
#include <string> // to_string

namespace mxstatd {

// то же самое, что и std::system_error, но с более удобным конструктором
class system_error
    : public std::system_error
{
public:
    system_error(int ec, const char* what_arg)
        : std::system_error(ec, std::system_category(), what_arg)
    {}
};

/// Класс-маркер для остановки EpollService::Run()
class epoll_service_terminate : public std::exception
{
public:
    epoll_service_terminate() {}
    const char* what() const noexcept override {
        return "epoll service terminate";
    }
};

} // namespace mxstatd
