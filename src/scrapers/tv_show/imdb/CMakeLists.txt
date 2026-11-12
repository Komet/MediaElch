add_library(
  mediaelch_scraper_tv_imdb OBJECT
  ImdbTv.cpp
  ImdbTvEpisodeScrapeJob.cpp
  ImdbTvSeasonScrapeJob.cpp
  ImdbTvShowScrapeJob.cpp
  ImdbTvShowSearchJob.cpp
  ImdbTvConfiguration.cpp
)

target_link_libraries(
  mediaelch_scraper_tv_imdb
  PRIVATE Qt${QT_VERSION_MAJOR}::Sql Qt${QT_VERSION_MAJOR}::Widgets
          Qt${QT_VERSION_MAJOR}::Multimedia Qt${QT_VERSION_MAJOR}::Xml
)
mediaelch_post_target_defaults(mediaelch_scraper_tv_imdb)
