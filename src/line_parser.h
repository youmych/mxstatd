#pragma once

#include <string_view>
#include <optional>
#include <cstdlib>

#include <data_log_event.h>

struct LineParser
{
    static std::optional<Data::Log::Event> Parse(std::string_view line) {
        constexpr auto npos = std::string_view::npos;

        auto openBracketPos = line.find('[');
        if( openBracketPos == npos )
            return {};

        auto closeBracketPos = line.find(']', openBracketPos);
        if( closeBracketPos == npos )
            return {};

        auto timestamp = line.substr(openBracketPos + 1, closeBracketPos - openBracketPos - 2);
        // это длинная метка времени, с датой. Она есть только в заголовке и в конце
        // к данным не относится
        if( npos != timestamp.find_first_of(" -.") )
            return {};
        // после скобок идут несколько табуляций. После них название события
        auto namePos = line.find_first_not_of("\t", closeBracketPos + 1);

        line.remove_prefix(namePos); // нам не нужны предыдущие символы
        line.remove_suffix(line.size() - line.find_last_not_of("\r\n\t")); // концевые тоже не нужны

        auto name = line.substr(0, line.find('\t'));
        auto tstr = line.substr( line.rfind('\t') + 1 );

        Data::Log::Event event{ Data::Log::EventType::UNKNOWN, 0 };
        // auto [unused, err] = std::from_chars(tstr.data(), tstr.data() + tstr.size(), event.TimeMs);
        event.TimeMs = std::atoi(tstr.data());

        if( name == "ORDER" ) {
            event.Type = Data::Log::EventType::ORDER;
        }
        return event;
    }
};
