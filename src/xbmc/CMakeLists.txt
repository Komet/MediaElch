add_library(mediaelch_xbmc OBJECT XbmcSync.cpp)

target_link_libraries(
  mediaelch_xbmc
  PRIVATE Qt5::Core Qt5::Multimedia Qt5::Widgets Qt5::Sql
)
mediaelch_post_target_defaults(mediaelch_xbmc)
