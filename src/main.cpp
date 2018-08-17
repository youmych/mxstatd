#include <iostream>
#include <exception>

#include <app_config.h>

int main(int argc, char** argv)
{
    try {
        if( !AppConfig::ParseArguments(argc, argv) )
            return 0;

        std::cout << "Input file: " << APP_CONFIG().InputFileName() << std::endl
            << "Input pipe: " << APP_CONFIG().InputPipeName() << std::endl
            << "Input TCP port: " << APP_CONFIG().InputTcpPort() << std::endl
            << "Listen UPD port: " << APP_CONFIG().OutputUdpPort() << std::endl
            << "Output file: " << APP_CONFIG().OutputFileName() << std::endl;
    }
    catch(std::exception& e) {
        std::cerr << "Caught exception: " << e.what() << std::endl;
        return -1;
    }

    return 0;
}

