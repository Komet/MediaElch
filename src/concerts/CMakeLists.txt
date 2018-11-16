add_library(
  mediaelch_concert
  OBJECT
  ConcertController.cpp
  ConcertFilesWidget.cpp
  ConcertSearch.cpp
  ConcertSearchWidget.cpp
  ConcertStreamDetailsWidget.cpp
  ConcertWidget.cpp
  ConcertInfoWidget.cpp
)

target_link_libraries(
  mediaelch_concert
  PRIVATE Qt5::Core Qt5::Widgets Qt5::Multimedia Qt5::Sql
)
mediaelch_post_target_defaults(mediaelch_concert)
