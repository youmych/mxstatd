#include <iostream>
#include <exception>
#include <fstream>
#include <future>

#include <cstring>
#include <cerrno>
#include <signal.h>
#include <execinfo.h>
#include <unistd.h>
#include <sys/types.h>

#include <app_config.h>
#include <thread_guard.h>
#include <line_parser.h>
#include <epoll_service.h>

#include <actor_tcp_listener.h>
#include <actor_named_pipe_reader.h>

// функция обработки сигналов
static void signal_handler(int sig)
{
    switch(sig) {
    case SIGUSR1:
        std::cout << "USR1" << std::endl;
        break;
    default:
        break;
    }
}

int main(int argc, char** argv)
{
    try {
        if( !AppConfig::ParseArguments(argc, argv) )
            return 0;

        // установим обработчик на SIGUSR1
        struct sigaction sigact;
        sigact.sa_flags = 0;
        sigact.sa_handler = signal_handler;
        sigemptyset(&sigact.sa_mask);
        sigaction(SIGUSR1, &sigact, 0);

        std::cout << "Input file: " << APP_CONFIG().InputFileName() << std::endl
            << "Input pipe: " << APP_CONFIG().InputPipeName() << std::endl
            << "Input TCP port: " << APP_CONFIG().InputTcpPort() << std::endl
            << "Listen UPD port: " << APP_CONFIG().OutputUdpPort() << std::endl
            << "Output file: " << APP_CONFIG().OutputFileName() << std::endl;

        std::cout << "My pid is: " << getpid() << std::endl;

        linux::io::EpollService eps;

        // цикл для работы с асинхронными событиями
        auto epsCycle = [&](){
            try {
                eps.Run();
            }
            catch(std::exception& e) {
                std::cout << "Exception in ept thread: " << e.what() << std::endl;
                eps.Stop();
            }
        };
        // цикл чтения из файла
        auto fileCycle = [&](auto path) {
            std::cout << "Input file: " << path << std::endl;
            std::ifstream file(path.c_str());

            std::string s;
            while( std::getline(file, s) ) {

                auto ev = LineParser::Parse(s);
                if( !ev ) {
                    std::cout << s << "\n";
                    continue;
                }
                if( ev.value().Type == Data::Log::EventType::UNKNOWN ) {
                    std::cout << s << "\n";
                    continue;
                }
                // work with event
            }
        };

        // блокируем сигналы, которые будем ждать для завершения программы
        sigset_t sigset;
        sigemptyset(&sigset);
        sigaddset(&sigset, SIGTERM);
        sigaddset(&sigset, SIGINT);
        sigaddset(&sigset, SIGQUIT);
        sigprocmask(SIG_BLOCK, &sigset, nullptr);

        if( APP_CONFIG().InputTcpPort() != 0 )
            eps.CreateActor<mxstatd::ActorTcpListener>(APP_CONFIG().InputTcpPort());
        if( !APP_CONFIG().InputPipeName().empty() )
            eps.CreateActor<mxstatd::ActorNamedPipeReader>(APP_CONFIG().InputPipeName());


        std::future<void> epsFuture = std::async(std::launch::async, std::move(epsCycle));
        std::future<void> fileFuture;
        if( !APP_CONFIG().InputFileName().empty() ) {
            fileFuture = std::async(std::launch::async, fileCycle, APP_CONFIG().InputFileName());
        }

        // ожидаем поступления сигнала
        for(;;) {
            siginfo_t siginfo;
            int rc = sigwaitinfo(&sigset, &siginfo);
            if( rc < 0 ) {
                if( errno == EINTR ) // сигнал уже обработан. Ложное срабатывание
                    continue;
                std::cout << "sigwaitinfo(): " << strerror(errno) << std::endl;
                break;
            }
            std::cout << "Received signal " << strsignal(siginfo.si_signo)
                << " (" << siginfo.si_signo  << ")."
                << std::endl;
            break;
        }

        // отправка сигналов о необходимости остановки
        eps.Stop();
        std::cout << "eps.Stop()" << std::endl;
        // завершение работы потоков
        if( fileFuture.valid() ) {
            fileFuture.wait();
        }
        epsFuture.wait();
    }
    catch(std::exception& e) {
        std::cerr << "Caught exception: " << e.what() << std::endl;
        return -1;
    }

    return 0;
}

