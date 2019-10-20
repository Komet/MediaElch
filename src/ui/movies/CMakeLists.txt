add_library(
  mediaelch_ui_movies OBJECT
  CertificationWidget.cpp
  GenreWidget.cpp
  MovieMultiScrapeDialog.cpp
  MovieSearchWidget.cpp
  MovieWidget.cpp
  MovieDuplicateItem.cpp
  MovieDuplicates.cpp
  MovieFilesWidget.cpp
  MovieSearch.cpp
)

target_link_libraries(
  mediaelch_ui_movies PRIVATE Qt5::Widgets Qt5::Core Qt5::Sql Qt5::Network
                              Qt5::Multimedia Qt5::MultimediaWidgets
)
mediaelch_post_target_defaults(mediaelch_ui_movies)
