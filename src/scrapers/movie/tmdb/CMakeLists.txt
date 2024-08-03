add_library(
  mediaelch_scraper_movie_tmdb OBJECT
  TmdbMovie.cpp TmdbMovieSearchJob.cpp TmdbMovieScrapeJob.cpp
  TmdbMovieConfiguration.cpp
)

target_link_libraries(
  mediaelch_scraper_movie_tmdb
  PRIVATE Qt${QT_VERSION_MAJOR}::Sql Qt${QT_VERSION_MAJOR}::Widgets
          Qt${QT_VERSION_MAJOR}::Multimedia Qt${QT_VERSION_MAJOR}::Xml
)
mediaelch_post_target_defaults(mediaelch_scraper_movie_tmdb)
