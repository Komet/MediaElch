add_library(mediaelch_scraper_omdb OBJECT OmdbApi.cpp)

target_link_libraries(
  mediaelch_scraper_omdb
  PRIVATE Qt${QT_VERSION_MAJOR}::Network Qt${QT_VERSION_MAJOR}::Widgets
)
mediaelch_post_target_defaults(mediaelch_scraper_omdb)
