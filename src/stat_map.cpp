#include <stat_map.h>
#include <statistics.h>

StatMap::stat_ptr_t
StatMap::GetStatistics(event_t eventType)
{
    // не считать статистику по инеизвестным типам событий
    if( eventType == Data::Log::EventType::UNKNOWN )
        return stat_ptr_t();

    std::lock_guard lock(m_MapAccessMutex);
    auto found = m_Map.find(eventType);
    if( found == m_Map.end() ) {
        auto stat = std::make_shared<Statistics>(Data::Log::Event::to_string(eventType));
        auto [it, ok] = m_Map.insert( std::make_pair(eventType, std::move(stat)) );
        if( ok )
            return it->second;
        else
            return stat_ptr_t();
    }
    return found->second;
}
