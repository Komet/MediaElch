add_library(mediaelch_scraper_scraper_tmdb OBJECT TmdbApi.cpp)

target_link_libraries(
  mediaelch_scraper_scraper_tmdb PRIVATE Qt5::Network Qt5::Core
)
mediaelch_post_target_defaults(mediaelch_scraper_scraper_tmdb)
