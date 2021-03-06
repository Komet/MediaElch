add_library(
  mediaelch_downloads OBJECT DownloadFileSearcher.cpp Extractor.cpp
                             FileWorker.cpp MakeMkvCon.cpp MyFile.cpp
)

target_link_libraries(
  mediaelch_downloads
  PRIVATE
    Qt${QT_VERSION_MAJOR}::Core Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Multimedia Qt${QT_VERSION_MAJOR}::Sql
    Qt${QT_VERSION_MAJOR}::Xml
)
mediaelch_post_target_defaults(mediaelch_downloads)
