add_library(
  mediaelch_concert
  OBJECT
  Concert.cpp
  ConcertController.cpp
  ConcertFileSearcher.cpp
  ConcertFilesWidget.cpp
  ConcertInfoWidget.cpp
  ConcertModel.cpp
  ConcertProxyModel.cpp
  ConcertSearch.cpp
  ConcertSearchWidget.cpp
  ConcertStreamDetailsWidget.cpp
  ConcertWidget.cpp
)

target_link_libraries(
  mediaelch_concert
  PRIVATE Qt5::Core Qt5::Widgets Qt5::Multimedia Qt5::Sql
)
mediaelch_post_target_defaults(mediaelch_concert)
