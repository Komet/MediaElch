add_library(
  mediaelch_ui_concerts OBJECT
  ConcertFilesWidget.cpp ConcertInfoWidget.cpp ConcertSearch.cpp
  ConcertSearchWidget.cpp ConcertStreamDetailsWidget.cpp ConcertWidget.cpp
)

target_link_libraries(
  mediaelch_ui_concerts PRIVATE Qt5::Widgets Qt5::Core Qt5::Sql Qt5::Network
                                Qt5::Multimedia Qt5::MultimediaWidgets
)
mediaelch_post_target_defaults(mediaelch_ui_concerts)
