add_library(
  mediaelch_scraper_tv_tvmaze OBJECT
  TvMaze.cpp
  TvMazeApi.cpp
  TvMazeEpisodeParser.cpp
  TvMazeShowSearchJob.cpp
  TvMazeShowParser.cpp
  TvMazeEpisodeScrapeJob.cpp
  TvMazeShowScrapeJob.cpp
  TvMazeSeasonScrapeJob.cpp
)

target_link_libraries(
  mediaelch_scraper_tv_tvmaze PRIVATE Qt5::Sql Qt5::Widgets Qt5::Multimedia
                                      Qt5::Xml
)
mediaelch_post_target_defaults(mediaelch_scraper_tv_tvmaze)
