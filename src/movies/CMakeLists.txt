add_library(
  mediaelch_movies
  OBJECT
  CertificationWidget.cpp
  GenreWidget.cpp
  Movie.cpp
  MovieController.cpp
  MovieCrew.cpp
  MovieDuplicateItem.cpp
  MovieDuplicates.cpp
  MovieFileSearcher.cpp
  MovieFilesOrganizer.cpp
  MovieFilesWidget.cpp
  MovieImages.cpp
  MovieModel.cpp
  MovieMultiScrapeDialog.cpp
  MovieProxyModel.cpp
  MovieSearch.cpp
  MovieSearchWidget.cpp
  MovieWidget.cpp
)

target_link_libraries(mediaelch_movies PRIVATE Qt5::Sql Qt5::MultimediaWidgets)
mediaelch_post_target_defaults(mediaelch_movies)
