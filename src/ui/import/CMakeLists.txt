add_library(
  mediaelch_ui_import OBJECT
  DownloadsWidget.cpp ImportActions.cpp ImportDialog.cpp MakeMkvDialog.cpp
  UnpackButtons.cpp
)

target_link_libraries(
  mediaelch_ui_import
  PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Xml
    Qt${QT_VERSION_MAJOR}::Sql
    Qt${QT_VERSION_MAJOR}::Network
    Qt${QT_VERSION_MAJOR}::Multimedia
    Qt${QT_VERSION_MAJOR}::MultimediaWidgets
)
mediaelch_post_target_defaults(mediaelch_ui_import)
