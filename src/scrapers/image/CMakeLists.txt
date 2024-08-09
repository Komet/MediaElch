add_library(
  mediaelch_image_providers OBJECT
  FanartTv.cpp
  FanartTvMusic.cpp
  FanartTvMusicArtists.cpp
  ImageProvider.cpp
  TheTvDbImages.cpp
  TmdbImages.cpp
  FanartTvConfiguration.cpp
  TheTvDbImagesConfiguration.cpp
)

target_link_libraries(
  mediaelch_image_providers
  PRIVATE
    Qt${QT_VERSION_MAJOR}::Core Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Multimedia Qt${QT_VERSION_MAJOR}::Xml
    Qt${QT_VERSION_MAJOR}::Sql
)
mediaelch_post_target_defaults(mediaelch_image_providers)
