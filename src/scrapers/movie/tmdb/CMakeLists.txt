add_library(mediaelch_scraper_movie_tmdb OBJECT TmdbMovie.cpp)

target_link_libraries(
  mediaelch_scraper_movie_tmdb PRIVATE Qt5::Sql Qt5::Widgets Qt5::Multimedia
                                       Qt5::Xml
)
mediaelch_post_target_defaults(mediaelch_scraper_movie_tmdb)
