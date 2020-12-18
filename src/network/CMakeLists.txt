add_library(
  mediaelch_network OBJECT
  HttpStatusCodes.cpp NetworkReplyWatcher.cpp NetworkRequest.cpp
  NetworkManager.cpp WebsiteCache.cpp
)

target_link_libraries(
  mediaelch_network PRIVATE Qt5::Core Qt5::Multimedia Qt5::Widgets Qt5::Sql
                            Qt5::Xml Qt5::MultimediaWidgets
)
mediaelch_post_target_defaults(mediaelch_network)
