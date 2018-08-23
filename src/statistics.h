#pragma once
#include <event_queue.h>
#include <data_log_event.h>

#include <map>
#include <cstdint>
#include <ostream>

class Statistics
{
    using counter_t = std::intmax_t;
    using map_t = std::map<int, counter_t>;
    using pair_t = map_t::value_type;

public:
    explicit Statistics() {}

    counter_t Total() const { return m_Total; }

    void AppendValue(int ms);

    std::ostream& PrintGeneric(std::ostream& os) const;
    std::ostream& PrintDetailed(std::ostream& os) const;

    std::ostream& PrintMaps(std::ostream& os) const;

private:
    /// Общее кол-во событий данного типа
    counter_t m_Total = 0;
    /// Частоты вхождения по каждому значению измерений
    map_t m_Freqs;
    /// Частоты вхожения по 5-мс диапазонам [x, x+5)
    map_t m_ClampedFreqs;
};
