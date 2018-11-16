add_library(
  mediaelch_imageProviders
  OBJECT
  FanartTv.cpp
  FanartTvMusic.cpp
  FanartTvMusicArtists.cpp
  TheTvDbImages.cpp
  TMDbImages.cpp
)

target_link_libraries(
  mediaelch_imageProviders
  PRIVATE Qt5::Core Qt5::Widgets Qt5::Multimedia Qt5::Xml
)
mediaelch_post_target_defaults(mediaelch_imageProviders)
