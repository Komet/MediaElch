add_library(
  mediaelch_scraper_tv_imdb OBJECT
  ImdbTv.cpp
  ImdbTvEpisodeParser.cpp
  ImdbTvEpisodeScrapeJob.cpp
  ImdbTvSeasonScrapeJob.cpp
  ImdbTvSeasonParser.cpp
  ImdbTvShowScrapeJob.cpp
  ImdbTvShowSearchJob.cpp
  ImdbTvShowParser.cpp
)

target_link_libraries(
  mediaelch_scraper_tv_imdb PRIVATE Qt5::Sql Qt5::Widgets Qt5::Multimedia
                                    Qt5::Xml
)
mediaelch_post_target_defaults(mediaelch_scraper_tv_imdb)
