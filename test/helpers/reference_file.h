#pragma once

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

} // namespace scraper
} // namespace test
