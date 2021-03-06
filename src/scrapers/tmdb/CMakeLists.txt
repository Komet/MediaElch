add_library(mediaelch_scraper_scraper_tmdb OBJECT TmdbApi.cpp)

target_link_libraries(
  mediaelch_scraper_scraper_tmdb PRIVATE Qt${QT_VERSION_MAJOR}::Network
                                         Qt${QT_VERSION_MAJOR}::Core
)
mediaelch_post_target_defaults(mediaelch_scraper_scraper_tmdb)
