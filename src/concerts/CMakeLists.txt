add_library(
  mediaelch_concert OBJECT
  Concert.cpp ConcertController.cpp ConcertFileSearcher.cpp ConcertModel.cpp
  ConcertProxyModel.cpp
)

target_link_libraries(
  mediaelch_concert
  PRIVATE Qt${QT_VERSION_MAJOR}::Core Qt${QT_VERSION_MAJOR}::Widgets
          Qt${QT_VERSION_MAJOR}::Multimedia Qt${QT_VERSION_MAJOR}::Sql
)
mediaelch_post_target_defaults(mediaelch_concert)
