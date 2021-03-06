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
  mediaelch_ui_movies
  PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Sql Qt${QT_VERSION_MAJOR}::Network
    Qt${QT_VERSION_MAJOR}::Multimedia Qt${QT_VERSION_MAJOR}::MultimediaWidgets
)
mediaelch_post_target_defaults(mediaelch_ui_movies)
