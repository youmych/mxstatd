#include <app_config.h>

#include <stdexcept>
#include <iostream>
#include <cstdlib>

#include <unistd.h>
#include <getopt.h>

//-----------------------------------------------------------------------------
bool AppConfig::ParseArguments(int argc, char** argv)
{
    static const struct option long_options[] = {
        { "file",   required_argument, nullptr, 'f' },
        { "socket", required_argument, nullptr, 's' },
        { "pipe",   required_argument, nullptr, 'p' },
        { "udp",    required_argument, nullptr, 'u' },
        { "outfile",required_argument, nullptr, 'o' },
        { "help",   no_argument,       nullptr, 'h' },
        { nullptr, 0, nullptr, 0 }
    };

    int option_index = 0;

    for(;;) {
        auto c = getopt_long(argc, argv, "f:s:p:u:oh",
                             long_options, &option_index);
        if (c == -1)
            break;

        switch (c) {
        case 0:
            break;

        case 'f': APP_CONFIG().m_InputFile = (optarg ? optarg : ""); break;
        case 's': APP_CONFIG().m_InputPort = std::atoi(optarg); break;
        case 'p': APP_CONFIG().m_InputPipeName = (optarg ? optarg : "");   break;
        case 'u': APP_CONFIG().m_UdpPort = std::atoi(optarg);   break;
        case 'o': APP_CONFIG().m_OutFileName = (optarg ? optarg : ""); break;

        case 'h': // show help
            Usage(argv[0]);
            return false;

        default:
            throw std::invalid_argument("invalid argument");
        }
    }
    return true;
}

//-----------------------------------------------------------------------------
void AppConfig::Usage(const char* appName)
{
    std::cout << "Usage:\n\t"
        << appName << " <options>\n\nOptions:\n"
        << "\t-f or --file path/to/file - input file name.\n"
        << "\t-s or --socket number - set tcp port number for input. Default is " << DEFAULT_TCP_PORT << "\n"
        << "\t-p or --pipe path/to/pipe - set path to input pipe.\n"
        << "\t-u or --udp number - set udp port number to listen. Default is " << DEFAULT_UDP_PORT << "\n"
        << "\t-o or --outfile path/to/file - path to report file. By default print to stdout.\n"
        << "\t-h or --help - show this help."
        << std::endl;
}
