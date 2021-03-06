add_library(
  mediaelch_network OBJECT
  HttpStatusCodes.cpp NetworkReplyWatcher.cpp NetworkRequest.cpp
  NetworkManager.cpp WebsiteCache.cpp
)

target_link_libraries(
  mediaelch_network
  PRIVATE
    Qt${QT_VERSION_MAJOR}::Core Qt${QT_VERSION_MAJOR}::Multimedia
    Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Sql
    Qt${QT_VERSION_MAJOR}::Xml Qt${QT_VERSION_MAJOR}::MultimediaWidgets
)

target_link_libraries(mediaelch_network PUBLIC Qt${QT_VERSION_MAJOR}::Network)
mediaelch_post_target_defaults(mediaelch_network)
