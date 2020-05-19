#include "tv_shows/SeasonOrder.h"

std::ostream& operator<<(std::ostream& os, const SeasonOrder& order)
{
    std::string str = [&order]() {
        switch (order) {
        case SeasonOrder::Aired: return "aired-order";
        case SeasonOrder::Dvd: return "dvd-order";
        }
    }();
    return os << str;
}
