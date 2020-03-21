add_library(
  mediaelch_ui_downloads OBJECT
  DownloadsWidget.cpp ImportActions.cpp ImportDialog.cpp MakeMkvDialog.cpp
  UnpackButtons.cpp
)

target_link_libraries(
  mediaelch_ui_downloads
  PRIVATE
    Qt5::Widgets
    Qt5::Core
    Qt5::Xml
    Qt5::Sql
    Qt5::Network
    Qt5::Multimedia
    Qt5::MultimediaWidgets
)
mediaelch_post_target_defaults(mediaelch_ui_downloads)
