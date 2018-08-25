#pragma once

#include <unordered_map>
#include <string>
#include <mutex>
#include <memory>

#include <data_log_event.h>
#include <statistics.h>

/**
 *  Статистики по разным типам сообщений
 */
class StatMap {

    StatMap() {};
    ~StatMap() {};
public:
    using event_t = Data::Log::EventType;
    using stat_ptr_t = std::shared_ptr<Statistics>;

    static StatMap& Instance() {
        static StatMap statMap;
        return statMap;
    }

    StatMap(const StatMap&) = delete;
    StatMap& operator=(const StatMap&) = delete;

    stat_ptr_t GetStatistics(event_t eventType);

private:
    std::mutex m_MapAccessMutex;
    std::unordered_map<event_t, stat_ptr_t> m_Map;
};

inline StatMap& STAT() {
    return StatMap::Instance();
}
