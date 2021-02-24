add_library(
  mediaelch_movies OBJECT
  Movie.cpp
  MovieController.cpp
  MovieCrew.cpp
  MovieFilesOrganizer.cpp
  MovieImages.cpp
  MovieModel.cpp
  MovieProxyModel.cpp
  MovieSet.cpp
  file_searcher/MovieFileSearcher.cpp
  file_searcher/MovieDirectorySearcher.cpp
)

target_link_libraries(
  mediaelch_movies PRIVATE Qt5::Sql Qt5::MultimediaWidgets Qt5::Concurrent
)
mediaelch_post_target_defaults(mediaelch_movies)
