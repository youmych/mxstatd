#pragma once
#include <string>
#include <string_view>

class BufferCutter
{
    std::string m_Buffer;
    size_t m_LineCount = 0;
    size_t m_Partials = 0;
public:
    explicit BufferCutter() {};

    size_t LineCount() const { return m_LineCount; }
    size_t Partials() const { return m_Partials; }

    template<class Notify>
    void operator()(std::string_view buf, Notify notify)
    {
        while( !buf.empty() ) {
            auto eolnPos = buf.find('\n');

            if( eolnPos == std::string_view::npos ) {
                m_Buffer.clear();
                std::copy(std::begin(buf), std::end(buf), std::back_inserter(m_Buffer));
                // std::cout << "Partial: '" << m_Buffer << "'" << std::endl;
                return;
            }
            m_LineCount++;

            if( m_Buffer.empty() ) {
                notify(buf.substr(0, eolnPos));
            }
            else {
                std::copy(std::begin(buf), std::begin(buf) + eolnPos, std::back_inserter(m_Buffer));
                // std::cout << "Tail: \"" << buf.substr(0, eolnPos) << "\"\n";
                notify(std::string_view(m_Buffer));
                m_Buffer.clear();
                m_Partials++;
            }
            buf.remove_prefix(std::min(buf.size(), eolnPos+1));
        }
    }
};
