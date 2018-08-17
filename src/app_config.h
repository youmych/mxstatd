#pragma once
#include <string>
#include <cstdint>

class AppConfig {
    explicit AppConfig() {}
    ~AppConfig() {}
public:
    static constexpr uint16_t DEFAULT_TCP_PORT = 9900;
    static constexpr uint16_t DEFAULT_UDP_PORT = 9000;

    AppConfig(AppConfig const&) = delete;
    AppConfig& operator=(AppConfig&) = delete;

    static AppConfig& Instance() {
        static AppConfig config;
        return config;
    }

    static bool ParseArguments(int argc, char** argv);
    static void Usage(const char* appName);

    const std::string& OutputFileName() const { return m_OutFileName; }
    const std::string& InputFileName() const { return m_InputFile; }
    const std::string& InputPipeName() const { return m_InputPipeName; }
    uint16_t InputTcpPort() const { return m_InputPort; }
    uint16_t OutputUdpPort() const { return m_UdpPort; }

private:
    std::string m_InputFile;
    std::string m_InputPipeName;
    uint16_t    m_InputPort = DEFAULT_TCP_PORT;
    uint16_t    m_UdpPort = DEFAULT_UDP_PORT;
    std::string m_OutFileName;
};

// helper
inline AppConfig& APP_CONFIG()
{
    return AppConfig::Instance();
}
