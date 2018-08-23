#pragma once
#include <event_queue.h>
#include <data_log_event.h>

#include <map>
#include <cstdint>
#include <ostream>
#include <vector>

class Statistics
{
    using counter_t = std::intmax_t;
    using map_t = std::map<int, counter_t>;
    using pair_t = map_t::value_type;

    struct StatItem {
        int ExecTime = 0;
        counter_t TransNo = 0;
        double Weigth = 0.0;
        double Percent = 0.0;

        explicit StatItem() {}
        explicit StatItem(int execTime, counter_t transNo, double weight, double percent)
            : ExecTime(execTime)
            , TransNo(transNo)
            , Weigth(weight)
            , Percent(percent)
        {}

        StatItem(const StatItem&) = default;
        StatItem& operator=(const StatItem&) = default;
    };

    struct StatGeneric {
        int Min = 0;
        int Mean = 0;
        int Less_90 = 0;
        int Less_99 = 0;
        int Less_99_9 = 0;
    };

    friend std::ostream& operator<<(std::ostream&, const StatItem&);
    friend std::ostream& operator<<(std::ostream&, const StatGeneric&);

public:
    explicit Statistics(const std::string& eventName)
        : m_EventName(eventName)
    {}

    counter_t Total() const { return m_Total; }

    void AppendValue(int ms);

    std::ostream& PrintGeneric(std::ostream& os) const;
    std::ostream& PrintDetailed(std::ostream& os) const;

    std::ostream& PrintMaps(std::ostream& os) const;

private:
    void UpdateDetailedCache() const;
    void UpdateGenericCache() const;

private:
    std::string m_EventName;
    /// Общее кол-во событий данного типа
    counter_t m_Total = 0;
    /// Частоты вхождения по каждому значению измерений
    map_t m_Freqs;
    /// Частоты вхожения по 5-мс диапазонам [x, x+5)
    map_t m_ClampedFreqs;
    /// Номер "поколения" данных для которого сгенерирован кеш
    mutable counter_t m_DetailedCaheGeneration = 0;
    mutable counter_t m_GeneriCacheGeneration = 0;
    /// Кеш статистики
    mutable std::vector<StatItem> m_DetailedStatCache;
    mutable StatGeneric           m_GenericStatCache;
};
