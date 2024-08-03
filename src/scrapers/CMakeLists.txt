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
  concert/ConcertIdentifier.cpp
  concert/ConcertScraper.cpp
  concert/ConcertSearchJob.cpp
  concert/tmdb/TmdbConcert.cpp
  concert/tmdb/TmdbConcertSearchJob.cpp
  movie/MovieIdentifier.cpp
  movie/MovieMerger.cpp
  movie/MovieScraper.cpp
  movie/MovieScrapeJob.cpp
  movie/MovieSearchJob.cpp
  music/MusicMerger.cpp
  music/AllMusic.cpp
  music/Discogs.cpp
  music/MusicBrainz.cpp
  music/MusicScraper.cpp
  music/TheAudioDb.cpp
  music/TvTunes.cpp
  music/UniversalMusicScraper.cpp
  ScraperError.cpp
  ScraperInfos.cpp
  ScraperInterface.cpp
  ScraperResult.cpp
  ScraperUtils.cpp
  ScraperConfiguration.cpp
  trailer/HdTrailers.cpp
  trailer/TrailerProvider.cpp
  trailer/TrailerResult.cpp
  tv_show/empty/EmptyTvScraper.cpp
  tv_show/EpisodeIdentifier.cpp
  tv_show/EpisodeScrapeJob.cpp
  tv_show/SeasonScrapeJob.cpp
  tv_show/ShowIdentifier.cpp
  tv_show/ShowMerger.cpp
  tv_show/ShowScrapeJob.cpp
  tv_show/ShowSearchJob.cpp
  tv_show/TvScraper.cpp
  TvShowUpdater.cpp
)

target_link_libraries(
  mediaelch_scrapers
  PRIVATE
    Qt${QT_VERSION_MAJOR}::Sql Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Multimedia Qt${QT_VERSION_MAJOR}::Xml
    Qt${QT_VERSION_MAJOR}::Network
)
mediaelch_post_target_defaults(mediaelch_scrapers)
