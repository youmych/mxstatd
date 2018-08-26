#include <statistics.h>
#include <iomanip>
#include <iterator>
#include <algorithm>

//-----------------------------------------------------------------------------
std::ostream& operator<<(std::ostream& os, const Statistics::StatItem& item)
{
    return os << item.ExecTime << '\t' << item.TransNo << '\t'
        << std::setprecision(4) << std::fixed << item.Weigth << '\t'
        << std::setprecision(4) << std::fixed << item.Percent;
}
//-----------------------------------------------------------------------------
std::ostream& operator<<(std::ostream& os, const Statistics::StatGeneric& stat)
{
    return os << "min=" << stat.Min
        << " 50%=" << stat.Mean
        << " 90%=" << stat.Less_90
        << " 99%=" << stat.Less_99
        << " 99.9%=" << stat.Less_99_9;
}
//-----------------------------------------------------------------------------
void Statistics::AppendValue(int ms)
{
    std::lock_guard lock(m_DataAccessMutex);
    DoAppendValue(ms);
}
//-----------------------------------------------------------------------------
void Statistics::DoAppendValue(int ms)
{
    static auto constexpr clampedBy = [](auto d) {
        return [=](auto v) constexpr {
            return (v - v % d);
        };
    };
    static auto clampedBy5ms = clampedBy(5);

    static auto append = [](auto& container, auto val) {
        auto it = container.find(val);
        if( it != container.end() ) {
            it->second++;
        }
        else {
            container[val] = 1;
        }
    };

    append(m_Freqs, ms);
    append(m_ClampedFreqs, clampedBy5ms(ms));
    m_Total++;
}
//-----------------------------------------------------------------------------
std::ostream& Statistics::PrintGeneric(std::ostream& os)
{
    // отладка
#if !defined(NDEBUG)
    std::unique_lock<std::shared_mutex> lock0(m_DataAccessMutex);
    std::vector<StatItem> newStat;
    newStat.reserve(m_Freqs.size());

    counter_t partSum = 0;
    for(const auto& [evTime, evCount]: m_Freqs) {
        partSum += evCount;
        newStat.emplace_back(evTime, evCount,
            (static_cast<double>(evCount) / m_Total) * 100.0,
            (static_cast<double>(partSum) / m_Total) * 100.0);
    }
    os << "Full stat:\nTIME\tTRANSNO\tWEIGHT\tPERCENT\n";
    std::copy(std::begin(newStat), std::end(newStat),
        std::ostream_iterator<Statistics::StatItem>(os, "\n"));
    lock0.unlock();
#endif

    UpdateGenericCache();

    os << m_EventName << " ";
    std::shared_lock<std::shared_mutex> lock(m_DataAccessMutex);
    os << m_GenericStatCache;
    lock.unlock();
    return os << std::endl;
}
//-----------------------------------------------------------------------------
std::ostream& Statistics::PrintDetailed(std::ostream& os)
{
    PrintGeneric(os);

    UpdateDetailedCache();

    os << "\nTIME\tTRANSNO\tWEIGHT\tPERCENT\n";
    std::shared_lock<std::shared_mutex> lock(m_DataAccessMutex);
    std::copy(std::begin(m_DetailedStatCache), std::end(m_DetailedStatCache),
        std::ostream_iterator<Statistics::StatItem>(os, "\n"));
    lock.unlock();
    return os;
}
//-----------------------------------------------------------------------------
std::ostream& Statistics::PrintMaps(std::ostream& os)
{
    for(auto& [key, value]: m_Freqs) {
        os << std::setw(4) << key << "\t" << std::setw(4) << value << "\n";
    }
    os << std::endl  << "ExecTime TransNo        Weight,%\n";
    for(auto& [key, value]: m_ClampedFreqs) {
        os << std::setw(4) << key << "\t"
            << std::setw(8) << value
            << "\t" << std::setw(8) << std::setprecision(4) << std::fixed
                << (static_cast<double>(value) / m_Total)*100.0
            << "\n";
    }
    return os;
}
//-----------------------------------------------------------------------------
void Statistics::UpdateDetailedCache()
{
    std::lock_guard<std::shared_mutex> lock(m_DataAccessMutex);

    if( 0 == m_Total || m_DetailedCaheGeneration == m_Total )
        return;

    std::vector<StatItem> newStat;
    newStat.reserve(m_ClampedFreqs.size());

    counter_t partSum = 0;
    for(const auto& [evTime, evCount]: m_ClampedFreqs) {
        partSum += evCount;
        newStat.emplace_back(evTime, evCount,
            (static_cast<double>(evCount) / m_Total) * 100.0,
            (static_cast<double>(partSum) / m_Total) * 100.0);
    }

    m_DetailedStatCache = std::move(newStat);
    m_DetailedCaheGeneration = m_Total;
}
//-----------------------------------------------------------------------------
void Statistics::UpdateGenericCache()
{
    std::lock_guard<std::shared_mutex> lock(m_DataAccessMutex);
    if( 0 == m_Total || m_DetailedCaheGeneration == m_Total )
        return;

    auto n50 = m_Total * 0.5;
    auto n90 = m_Total * 0.9;
    auto n99 = m_Total * 0.99;
    auto n99_9 = m_Total * 0.999;

    auto it = m_Freqs.begin();
    m_GenericStatCache.Min = it->first;

    counter_t partSum = 0;
    for( ; it != m_Freqs.end(); ++it ) {
        if( n50 <= partSum ) {
            m_GenericStatCache.Mean = it->first;
            break;
        }
        partSum += it->second;
    }
    for( ; it != m_Freqs.end(); ++it ) {
        if( n90 <= partSum ) {
            m_GenericStatCache.Less_90 = it->first;
            break;
        }
        partSum += it->second;
    }
    for( ; it != m_Freqs.end(); ++it ) {
        if( n99 <= partSum ) {
            m_GenericStatCache.Less_99 = it->first;
            break;
        }
        partSum += it->second;
    }
    for( ; it != m_Freqs.end(); ++it ) {
        if( n99_9 <= partSum ) {
            m_GenericStatCache.Less_99_9 = it->first;
            break;
        }
        partSum += it->second;
    }

    m_GeneriCacheGeneration = m_Total;
}
