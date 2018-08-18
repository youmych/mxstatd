#pragma once

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
            case EventType::ORDER: return "ORDER";
            case EventType::UNKNOWN: return "UNKNOWN";
        }
    }
};

} // namespace Log
} // namespace Data
