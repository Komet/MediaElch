#include "tv_shows/SeasonOrder.h"

#include <QDebugStateSaver>

static const char* seasonOrderToString(SeasonOrder order)
{
    switch (order) {
    case SeasonOrder::Aired: return "aired-order";
    case SeasonOrder::Dvd: return "dvd-order";
    }
    // should not happen but still return the default
    return "aired-order";
}

std::ostream& operator<<(std::ostream& os, SeasonOrder order)
{
    return os << seasonOrderToString(order);
}

QDebug operator<<(QDebug debug, SeasonOrder order)
{
    QDebugStateSaver saver(debug);
    debug.nospace() << "SeasonOrder(" << seasonOrderToString(order) << ')';
    return debug;
}
