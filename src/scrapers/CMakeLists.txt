add_subdirectory(image)
add_subdirectory(tv_show)

add_library(
  mediaelch_scrapers OBJECT
  # Headers so that moc is run on them
  concert/ConcertScraper.h
  movie/MovieScraper.h
  music/MusicScraper.h
  tv_show/TvScraper.h
  # Sources
  ScraperInterface.cpp
  ScraperError.cpp
  concert/ConcertIdentifier.cpp
  concert/ConcertScraper.cpp
  concert/TmdbConcert.cpp
  movie/MovieIdentifier.cpp
  movie/AdultDvdEmpire.cpp
  movie/AEBN.cpp
  movie/CustomMovieScraper.cpp
  movie/HotMovies.cpp
  movie/IMDB.cpp
  movie/imdb/ImdbMovieScraper.cpp
  movie/MovieScraper.cpp
  movie/OFDb.cpp
  movie/TMDb.cpp
  movie/VideoBuster.cpp
  music/TvTunes.cpp
  music/UniversalMusicScraper.cpp
  music/MusicScraper.cpp
  trailer/HdTrailers.cpp
  trailer/TrailerProvider.cpp
  tv_show/ShowMerger.cpp
  tv_show/ShowIdentifier.cpp
  tv_show/EpisodeIdentifier.cpp
  tv_show/TvScraper.cpp
  tv_show/EpisodeScrapeJob.cpp
  tv_show/ShowScrapeJob.cpp
  tv_show/ShowSearchJob.cpp
  tv_show/SeasonScrapeJob.cpp
  tv_show/empty/EmptyTvScraper.cpp
)

target_link_libraries(
  mediaelch_scrapers PRIVATE Qt5::Sql Qt5::Widgets Qt5::Multimedia Qt5::Xml
)
mediaelch_post_target_defaults(mediaelch_scrapers)
