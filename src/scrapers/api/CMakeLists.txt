add_library(mediaelch_scraper_scraper_apis OBJECT TmdbTvApi.cpp)

target_link_libraries(
  mediaelch_scraper_scraper_apis PRIVATE Qt5::Network Qt5::Core
)
mediaelch_post_target_defaults(mediaelch_scraper_scraper_apis)
