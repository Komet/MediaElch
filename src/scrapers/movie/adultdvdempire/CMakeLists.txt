add_library(
  mediaelch_scraper_movie_ade OBJECT
  AdultDvdEmpire.cpp AdultDvdEmpireApi.cpp AdultDvdEmpireSearchJob.cpp
  AdultDvdEmpireScrapeJob.cpp AdultDvdEmpireConfiguration.cpp
)

target_link_libraries(
  mediaelch_scraper_movie_ade
  PRIVATE Qt${QT_VERSION_MAJOR}::Sql Qt${QT_VERSION_MAJOR}::Widgets
          Qt${QT_VERSION_MAJOR}::Multimedia Qt${QT_VERSION_MAJOR}::Xml
)
mediaelch_post_target_defaults(mediaelch_scraper_movie_ade)
