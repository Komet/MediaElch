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
  mediaelch_movies
  PRIVATE Qt${QT_VERSION_MAJOR}::Sql Qt${QT_VERSION_MAJOR}::MultimediaWidgets
          Qt${QT_VERSION_MAJOR}::Concurrent Qt${QT_VERSION_MAJOR}::Svg
)
mediaelch_post_target_defaults(mediaelch_movies)
