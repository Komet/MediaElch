add_library(
  mediaelch_import OBJECT MyFile.cpp Extractor.cpp MakeMkvCon.cpp
                          DownloadFileSearcher.cpp FileWorker.cpp
)

target_link_libraries(
  mediaelch_import
  PRIVATE Qt${QT_VERSION_MAJOR}::Core Qt${QT_VERSION_MAJOR}::Multimedia
          Qt${QT_VERSION_MAJOR}::Widgets
)
mediaelch_post_target_defaults(mediaelch_import)
