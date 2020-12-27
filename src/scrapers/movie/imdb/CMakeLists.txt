add_library(
  mediaelch_scraper_movie_imdb OBJECT ImdbMovie.cpp ImdbMovieScraper.cpp
)

target_link_libraries(
  mediaelch_scraper_movie_imdb PRIVATE Qt5::Sql Qt5::Widgets Qt5::Multimedia
                                       Qt5::Xml
)
mediaelch_post_target_defaults(mediaelch_scraper_movie_imdb)
