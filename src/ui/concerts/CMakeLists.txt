add_library(
  mediaelch_ui_concerts OBJECT
  ConcertFilesWidget.cpp ConcertInfoWidget.cpp ConcertSearch.cpp
  ConcertSearchWidget.cpp ConcertStreamDetailsWidget.cpp ConcertWidget.cpp
)

target_link_libraries(
  mediaelch_ui_concerts
  PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Sql Qt${QT_VERSION_MAJOR}::Network
    Qt${QT_VERSION_MAJOR}::Multimedia Qt${QT_VERSION_MAJOR}::MultimediaWidgets
)
mediaelch_post_target_defaults(mediaelch_ui_concerts)
