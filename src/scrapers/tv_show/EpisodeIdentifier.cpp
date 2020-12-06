#include "scrapers/tv_show/EpisodeIdentifier.h"

namespace mediaelch {
namespace scraper {

QDebug operator<<(QDebug debug, const EpisodeIdentifier& id)
{
    QDebugStateSaver saver(debug);
    if (id.hasEpisodeIdentifier()) {
        debug.nospace() << "EpisodeIdentifier(" << id.episodeIdentifier << ')';
    } else {
        debug.nospace() << "EpisodeIdentifier(Show(" << id.showIdentifier << "), " << id.seasonNumber << ", "
                        << id.episodeNumber << ')';
    }
    return debug;
}


} // namespace scraper
} // namespace mediaelch
