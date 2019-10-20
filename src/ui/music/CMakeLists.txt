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
  mediaelch_ui_music PRIVATE Qt5::Widgets Qt5::Sql Qt5::Network Qt5::Xml
                             Qt5::Concurrent Qt5::OpenGL
)
mediaelch_post_target_defaults(mediaelch_ui_music)
