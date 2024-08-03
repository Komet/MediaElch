add_library(
  mediaelch_scraper_tv_thetvdb OBJECT
  TheTvDb.cpp
  TheTvDbApi.cpp
  TheTvDbEpisodeParser.cpp
  TheTvDbEpisodesParser.cpp
  TheTvDbShowSearchJob.cpp
  TheTvDbShowParser.cpp
  TheTvDbEpisodeScrapeJob.cpp
  TheTvDbShowScrapeJob.cpp
  TheTvDbSeasonScrapeJob.cpp
  TheTvDbConfiguration.cpp
)

target_link_libraries(
  mediaelch_scraper_tv_thetvdb
  PRIVATE Qt${QT_VERSION_MAJOR}::Sql Qt${QT_VERSION_MAJOR}::Widgets
          Qt${QT_VERSION_MAJOR}::Multimedia Qt${QT_VERSION_MAJOR}::Xml
)
mediaelch_post_target_defaults(mediaelch_scraper_tv_thetvdb)
