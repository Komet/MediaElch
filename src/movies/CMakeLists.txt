add_library(
  mediaelch_movies OBJECT
  Movie.cpp
  MovieController.cpp
  MovieCrew.cpp
  MovieFileSearcher.cpp
  MovieFilesOrganizer.cpp
  MovieImages.cpp
  MovieModel.cpp
  MovieProxyModel.cpp
  MovieSet.cpp
)

target_link_libraries(mediaelch_movies PRIVATE Qt5::Sql Qt5::MultimediaWidgets)
mediaelch_post_target_defaults(mediaelch_movies)
