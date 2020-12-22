#include "scrapers/movie/MovieIdentifier.h"

namespace mediaelch {
namespace scraper {

QDebug operator<<(QDebug debug, const MovieIdentifier& id)
{
    QDebugStateSaver saver(debug);
    debug.nospace() << "MovieIdentifier(" << id.str() << ')';

    return debug;
}

} // namespace scraper
} // namespace mediaelch
