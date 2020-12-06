#include "scrapers/tv_show/ShowIdentifier.h"

namespace mediaelch {
namespace scraper {

QDebug operator<<(QDebug debug, const ShowIdentifier& id)
{
    QDebugStateSaver saver(debug);
    debug.nospace() << "ShowIdentifier(" << id.str() << ')';

    return debug;
}

} // namespace scraper
} // namespace mediaelch
