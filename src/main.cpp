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
#include <actor_udp_server.h>

#include <event_queue.h>
#include <statistics.h>
#include <stat_map.h>

#if 0
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
#endif

int main(int argc, char** argv)
{
    try {
        if( !AppConfig::ParseArguments(argc, argv) )
            return 0;
#if 0
        // установим обработчик на SIGUSR1
        struct sigaction sigact;
        sigact.sa_flags = 0;
        sigact.sa_handler = signal_handler;
        sigemptyset(&sigact.sa_mask);
        sigaction(SIGUSR1, &sigact, 0);
#endif
        // блокируем сигналы, которые будем ждать для завершения программы
        sigset_t sigset;
        sigemptyset(&sigset);
        sigaddset(&sigset, SIGTERM);
        sigaddset(&sigset, SIGINT);
        sigaddset(&sigset, SIGQUIT);
        sigaddset(&sigset, SIGUSR1);
        sigprocmask(SIG_BLOCK, &sigset, nullptr);

        // std::cout << "Input file: " << APP_CONFIG().InputFileName() << std::endl
        //     << "Input pipe: " << APP_CONFIG().InputPipeName() << std::endl
        //     << "Input TCP port: " << APP_CONFIG().InputTcpPort() << std::endl
        //     << "Listen UPD port: " << APP_CONFIG().OutputUdpPort() << std::endl
        //     << "Output file: " << APP_CONFIG().OutputFileName() << std::endl;

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
        std::atomic<bool> fileThreadDone { false };
        auto fileCycle = [&](auto path) {
            std::cout << "Input file: " << path << std::endl;
            std::ifstream file(path.c_str());
            auto collector = EVENT_QUEUE().MakeCollector();

            std::string s;
            size_t total_ = 0;
            while( std::getline(file, s) ) {
                if( fileThreadDone.load() )
                    break;
                total_++;
                auto ev = LineParser::Parse(s);
                if( !ev ) {
                    continue;
                }
                if( ev.value().Type == Data::Log::EventType::UNKNOWN ) {
                    continue;
                }
                // work with event
                collector->AppendEvent(ev.value());
                // std::this_thread::sleep_for(std::chrono::microseconds(10));
            }
            std::cout << "End of file " << path << ". Processed " << total_ << " lines." << std::endl;
        };

        EVENT_QUEUE().Start();

        if( APP_CONFIG().InputTcpPort() != 0 )
            eps.CreateActor<mxstatd::ActorTcpListener>(APP_CONFIG().InputTcpPort());
        if( !APP_CONFIG().InputPipeName().empty() )
            eps.CreateActor<mxstatd::ActorNamedPipeReader>(APP_CONFIG().InputPipeName());

        eps.CreateActor<mxstatd::ActorUdpServer>(APP_CONFIG().OutputUdpPort());

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
            if( siginfo.si_signo == SIGUSR1 ) {
                if( !APP_CONFIG().OutputFileName().empty() ) {
                    std::ofstream outf(APP_CONFIG().OutputFileName().c_str());
                    if( outf.is_open() )
                        STAT().GetStatistics(Data::Log::EventType::ORDER)->PrintDetailed(outf);
                }
                else
                    STAT().GetStatistics(Data::Log::EventType::ORDER)->PrintDetailed(std::cout);
                continue;
            }
            break;
        }

        // отправка сигналов о необходимости остановки
        eps.Stop();
        fileThreadDone.store(true);
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

