add_library(
  mediaelch_scraper_tv_tmdb OBJECT
  TmdbTv.cpp
  TmdbTvEpisodeParser.cpp
  TmdbTvSeasonParser.cpp
  TmdbTvEpisodeScrapeJob.cpp
  TmdbTvSeasonScrapeJob.cpp
  TmdbTvShowScrapeJob.cpp
  TmdbTvShowSearchJob.cpp
  TmdbTvShowParser.cpp
  TmdbTvConfiguration.cpp
)

target_link_libraries(
  mediaelch_scraper_tv_tmdb
  PRIVATE
    mediaelch_scraper_scraper_tmdb Qt${QT_VERSION_MAJOR}::Sql
    Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Multimedia
    Qt${QT_VERSION_MAJOR}::Xml
)
mediaelch_post_target_defaults(mediaelch_scraper_tv_tmdb)
