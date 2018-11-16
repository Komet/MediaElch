add_library(
  mediaelch_movies
  OBJECT
  CertificationWidget.cpp
  GenreWidget.cpp
  MovieController.cpp
  MovieDuplicateItem.cpp
  MovieDuplicates.cpp
  MovieFilesWidget.cpp
  MovieMultiScrapeDialog.cpp
  MovieSearch.cpp
  MovieSearchWidget.cpp
  MovieWidget.cpp
)

target_link_libraries(mediaelch_movies PRIVATE Qt5::Sql Qt5::MultimediaWidgets)
mediaelch_post_target_defaults(mediaelch_movies)
