#include <statistics.h>
#include <iomanip>

//-----------------------------------------------------------------------------
void Statistics::AppendValue(int ms)
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
std::ostream& Statistics::PrintGeneric(std::ostream& os) const
{
    return os;
}
//-----------------------------------------------------------------------------
std::ostream& Statistics::PrintDetailed(std::ostream& os) const
{
    return os;
}
//-----------------------------------------------------------------------------
std::ostream& Statistics::PrintMaps(std::ostream& os) const
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
