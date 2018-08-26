#pragma once

#include <cstring>

namespace Data {
namespace Log {

/// Типы записей в логе
enum class EventType {
    ORDER,  /// тип ORDER
    UNKNOWN ///< "все другие"
};

struct Event {
    EventType Type;
    int TimeMs;

    static const char* to_string(EventType evt) {
        switch(evt) {
            case EventType::ORDER:
                return "ORDER";
            case EventType::UNKNOWN:
            default:
                return "UNKNOWN";
        }
    }

    static EventType from_string(const char* text, size_t len = 0) {
        if( !text )
            return EventType::UNKNOWN;

        if( len == 0 )
            len = strlen(text);

        if( (len = 5) && (0 == strncmp(text, "ORDER", len)) )
            return EventType::ORDER;
        else // if( ... ) else if ( ... ) ...
            return EventType::UNKNOWN;
    }
};

} // namespace Log
} // namespace Data
