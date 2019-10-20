add_subdirectory(image)

add_library(
  mediaelch_scrapers OBJECT
  # Headers so that moc is run on them
  concert/ConcertScraperInterface.h
  movie/MovieScraperInterface.h
  music/MusicScraperInterface.h
  tv_show/TvScraperInterface.h
  # Sources
  concert/TMDbConcerts.cpp
  movie/AdultDvdEmpire.cpp
  movie/AEBN.cpp
  movie/CustomMovieScraper.cpp
  movie/HotMovies.cpp
  movie/IMDB.cpp
  movie/OFDb.cpp
  movie/TMDb.cpp
  movie/VideoBuster.cpp
  music/TvTunes.cpp
  music/UniversalMusicScraper.cpp
  trailer/HdTrailers.cpp
  trailer/TrailerProvider.cpp
  tv_show/TheTvDb.cpp
)

target_link_libraries(
  mediaelch_scrapers PRIVATE Qt5::Sql Qt5::Widgets Qt5::Multimedia Qt5::Xml
)
mediaelch_post_target_defaults(mediaelch_scrapers)
