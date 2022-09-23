add_library(
  mediaelch_network OBJECT
  WebsiteCache.cpp
  NetworkRequest.cpp
  NetworkManager.cpp
  DownloadManager.cpp
  HttpStatusCodes.cpp
  DownloadManagerElement.cpp
  NetworkReplyWatcher.cpp
)

target_link_libraries(
  mediaelch_network
  PRIVATE Qt${QT_VERSION_MAJOR}::Core Qt${QT_VERSION_MAJOR}::Network
          Qt${QT_VERSION_MAJOR}::Multimedia
)
mediaelch_post_target_defaults(mediaelch_network)
