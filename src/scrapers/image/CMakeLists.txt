add_library(
  mediaelch_image_providers OBJECT
  FanartTv.cpp FanartTvMusic.cpp FanartTvMusicArtists.cpp
  ImageProviderInterface.cpp TheTvDbImages.cpp TMDbImages.cpp
)

target_link_libraries(
  mediaelch_image_providers PRIVATE Qt5::Core Qt5::Widgets Qt5::Multimedia
                                    Qt5::Xml
)
mediaelch_post_target_defaults(mediaelch_image_providers)
