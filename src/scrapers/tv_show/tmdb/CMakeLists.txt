add_library(
  mediaelch_scraper_tv_tmdb OBJECT
  TmdbTv.cpp
  TmdbTvApi.cpp
  TmdbTvEpisodeParser.cpp
  TmdbTvSeasonParser.cpp
  TmdbTvEpisodeScrapeJob.cpp
  TmdbTvSeasonScrapeJob.cpp
  TmdbTvShowScrapeJob.cpp
  TmdbTvShowSearchJob.cpp
  TmdbTvShowParser.cpp
)

target_link_libraries(
  mediaelch_scraper_tv_tmdb PRIVATE Qt5::Sql Qt5::Widgets Qt5::Multimedia
                                    Qt5::Xml
)
mediaelch_post_target_defaults(mediaelch_scraper_tv_tmdb)
