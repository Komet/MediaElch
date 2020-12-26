add_library(mediaelch_scraper_movie_ofdb OBJECT OFDb.cpp OfdbApi.cpp)

target_link_libraries(
  mediaelch_scraper_movie_ofdb PRIVATE Qt5::Sql Qt5::Widgets Qt5::Multimedia
                                       Qt5::Xml
)
mediaelch_post_target_defaults(mediaelch_scraper_movie_ofdb)
