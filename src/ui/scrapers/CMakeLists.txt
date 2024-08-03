add_library(
  mediaelch_ui_scrapers OBJECT
  ScraperManager.cpp
  movie/AebnConfigurationView.cpp
  movie/TmdbMovieConfigurationView.cpp
  movie/ImdbMovieConfigurationView.cpp
  tv_show/FernsehserienDeConfigurationView.cpp
  tv_show/ImdbTvConfigurationView.cpp
  tv_show/TheTvDbConfigurationView.cpp
  tv_show/TmdbTvConfigurationView.cpp
  tv_show/TvMazeConfigurationView.cpp
  music/UniversalMusicConfigurationView.cpp
  concert/TmdbConcertConfigurationView.cpp
  image/FanartTvConfigurationView.cpp
)

target_link_libraries(
  mediaelch_ui_scrapers
  PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Network
          Qt${QT_VERSION_MAJOR}::Sql Qt${QT_VERSION_MAJOR}::Xml
)
mediaelch_post_target_defaults(mediaelch_ui_scrapers)
