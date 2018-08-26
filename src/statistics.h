#pragma once
#include <event_queue.h>
#include <data_log_event.h>

#include <map>
#include <cstdint>
#include <ostream>
#include <vector>
#include <algorithm>
#include <mutex>
#include <shared_mutex>

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

    counter_t Total() const {
        std::shared_lock<std::shared_mutex> lock(m_DataAccessMutex);
        return m_Total;
    }

    void AppendValue(int ms);

    template <class Iterator, class Adaptor>
    void AppendValues(Iterator begin, Iterator end, Adaptor adaptor) {
        std::lock_guard<std::shared_mutex> lock(m_DataAccessMutex);
        std::for_each(begin, end, [&,this](auto v){
            DoAppendValue(adaptor(v));
        });
    }

    template <class Iterator>
    void AppendValues(Iterator begin, Iterator end) {
        std::lock_guard<std::shared_mutex> lock(m_DataAccessMutex);
        std::for_each(begin, end, [this](auto v){
            DoAppendValue(v);
        });
    }

    template <class Container, class Adaptor>
    void AppendValues(const Container& c, Adaptor adapt) {
        std::lock_guard<std::shared_mutex> lock(m_DataAccessMutex);
        std::for_each(std::begin(c), std::end(c), [&,this](auto v){
            DoAppendValue(adapt(v));
        });
    }

    std::ostream& PrintGeneric(std::ostream& os);
    std::ostream& PrintDetailed(std::ostream& os);

    std::ostream& PrintMaps(std::ostream& os);

private:
    void UpdateDetailedCache();
    void UpdateGenericCache();
    void DoAppendValue(int ms);

private:
    std::string m_EventName;
    /// Общее кол-во событий данного типа
    counter_t m_Total = 0;
    /// Частоты вхождения по каждому значению измерений
    map_t m_Freqs;
    /// Частоты вхожения по 5-мс диапазонам [x, x+5)
    map_t m_ClampedFreqs;
    /// Номер "поколения" данных для которого сгенерирован кеш
    counter_t m_DetailedCaheGeneration = 0;
    counter_t m_GeneriCacheGeneration = 0;
    /// Кеш статистики
    std::vector<StatItem> m_DetailedStatCache;
    StatGeneric           m_GenericStatCache;

    mutable std::shared_mutex m_DataAccessMutex;
};
