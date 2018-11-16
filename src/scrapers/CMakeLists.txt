add_library(
  mediaelch_scrapers
  OBJECT
  AdultDvdEmpire.cpp
  AEBN.cpp
  CustomMovieScraper.cpp
  HotMovies.cpp
  IMDB.cpp
  OFDb.cpp
  TheTvDb.cpp
  TMDb.cpp
  TMDbConcerts.cpp
  TvTunes.cpp
  UniversalMusicScraper.cpp
  VideoBuster.cpp
)

target_link_libraries(
  mediaelch_scrapers
  PRIVATE Qt5::Sql Qt5::Widgets Qt5::Multimedia Qt5::Xml
)
mediaelch_post_target_defaults(mediaelch_scrapers)
