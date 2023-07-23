#pragma once

#include "scrapers/ScraperInfos.h"

#include <QSet>

class Artist;
class Album;

namespace mediaelch {
namespace scraper {

void copyDetailsToArtist(Artist& target, const Artist& source, const QSet<MusicScraperInfo>& details);
void copyDetailsToAlbum(Album& target, const Album& source, const QSet<MusicScraperInfo>& details);

/// \brief Same as copyDetailsToArtist(), but only copies the details if the target does not have it set.
/// \todo: Remove: should not be necessary or the default
void copyDetailsToArtistIfEmpty(Artist& target, const Artist& source, const QSet<MusicScraperInfo>& details);
/// \brief Same as copyDetailsToAlbum(), but only copies the details if the target does not have it set.
/// \todo: Remove: should not be necessary or the default
void copyDetailsToAlbumIfEmpty(Album& target, const Album& source, const QSet<MusicScraperInfo>& details);

} // namespace scraper
} // namespace mediaelch
