#pragma once

#include "data/tv_show/EpisodeMap.h"

#include <QString>

class Album;
class Artist;
class Concert;
class Movie;
class TvShow;
class TvShowEpisode;

namespace test {
namespace scraper {

QString serializeForReference(Album& album);
QString serializeForReference(Artist& album);
QString serializeForReference(Concert& concert);
QString serializeForReference(Movie& concert);
QString serializeForReference(TvShow& show);
QString serializeForReference(TvShowEpisode& episode);
QString serializeForReference(const mediaelch::EpisodeMap& episodes);

} // namespace scraper
} // namespace test
