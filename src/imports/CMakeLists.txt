add_library(
  mediaelch_downloads OBJECT Extractor.cpp FileWorker.cpp MakeMkvCon.cpp
                             MyFile.cpp
)

target_link_libraries(
  mediaelch_downloads PRIVATE Qt5::Core Qt5::Widgets Qt5::Multimedia Qt5::Sql
                              Qt5::Xml
)
mediaelch_post_target_defaults(mediaelch_downloads)
