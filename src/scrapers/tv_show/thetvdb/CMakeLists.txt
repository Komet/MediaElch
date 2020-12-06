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
)

target_link_libraries(
  mediaelch_scraper_tv_thetvdb PRIVATE Qt5::Sql Qt5::Widgets Qt5::Multimedia
                                       Qt5::Xml
)
mediaelch_post_target_defaults(mediaelch_scraper_tv_thetvdb)
