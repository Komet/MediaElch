add_library(
  mediaelch_ui_shows OBJECT
  TvShowSearch.cpp
  TvShowSearchWidget.cpp
  TvShowCommonWidgets.cpp
  TvShowWidget.cpp
  TvShowWidgetEpisode.cpp
  TvShowWidgetSeason.cpp
  TvShowWidgetTvShow.cpp
  TvTunesDialog.cpp
  TvShowMultiScrapeDialog.cpp
)

target_link_libraries(
  mediaelch_ui_shows
  PRIVATE
    Qt5::Widgets
    Qt5::Core
    Qt5::Xml
    Qt5::Sql
    Qt5::Network
    Qt5::Multimedia
    Qt5::MultimediaWidgets
)
mediaelch_post_target_defaults(mediaelch_ui_shows)
