#include "scrapers/concert/ConcertIdentifier.h"

namespace mediaelch {
namespace scraper {

QDebug operator<<(QDebug debug, const ConcertIdentifier& id)
{
    QDebugStateSaver saver(debug);
    debug.nospace() << "ConcertIdentifier(" << id.str() << ')';

    return debug;
}

} // namespace scraper
} // namespace mediaelch
