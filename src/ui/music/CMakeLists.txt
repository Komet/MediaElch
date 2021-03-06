add_library(
  mediaelch_ui_music OBJECT
  MusicFilesWidget.cpp
  MusicMultiScrapeDialog.cpp
  MusicSearch.cpp
  MusicSearchWidget.cpp
  MusicWidget.cpp
  MusicWidgetAlbum.cpp
  MusicWidgetArtist.cpp
)
target_link_libraries(
  mediaelch_ui_music
  PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Sql
    Qt${QT_VERSION_MAJOR}::Network Qt${QT_VERSION_MAJOR}::Xml
    Qt${QT_VERSION_MAJOR}::Concurrent Qt${QT_VERSION_MAJOR}::OpenGL
)
mediaelch_post_target_defaults(mediaelch_ui_music)
