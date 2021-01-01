add_subdirectory(tmdb)
add_subdirectory(imdb)
add_subdirectory(image)
add_subdirectory(movie)
add_subdirectory(tv_show)

add_library(
  mediaelch_scrapers OBJECT
  # Headers so that moc is run on them
  music/MusicScraper.h
  # Sources
  ScraperInterface.cpp
  ScraperError.cpp
  concert/ConcertIdentifier.cpp
  concert/ConcertScraper.cpp
  concert/ConcertSearchJob.cpp
  concert/tmdb/TmdbConcert.cpp
  concert/tmdb/TmdbConcertSearchJob.cpp
  movie/MovieIdentifier.cpp
  movie/MovieScraper.cpp
  music/TvTunes.cpp
  music/AllMusic.cpp
  music/UniversalMusicScraper.cpp
  music/MusicBrainz.cpp
  music/TheAudioDb.cpp
  music/MusicScraper.cpp
  music/Discogs.cpp
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
