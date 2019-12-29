add_library(
  mediaelch_globals OBJECT
  Actor.cpp
  ComboDelegate.cpp
  DownloadManager.cpp
  DownloadManagerElement.cpp
  Filter.cpp
  Globals.cpp
  Helper.cpp
  ImageDialog.cpp
  ImagePreviewDialog.cpp
  JsonRequest.cpp
  Manager.cpp
  MessageIds.cpp
  NameFormatter.cpp
  NetworkReplyWatcher.cpp
  Poster.cpp
  ScraperInfos.cpp
  ScraperResult.cpp
  TrailerDialog.cpp
  VersionInfo.cpp
)

target_link_libraries(
  mediaelch_globals PRIVATE Qt5::Core Qt5::Multimedia Qt5::Widgets Qt5::Sql
                            Qt5::Xml Qt5::MultimediaWidgets
)
mediaelch_post_target_defaults(mediaelch_globals)
