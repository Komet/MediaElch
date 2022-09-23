add_subdirectory(movies)
add_subdirectory(concerts)
add_subdirectory(small_widgets)
add_subdirectory(tv_show)
add_subdirectory(export)
add_subdirectory(image)
add_subdirectory(imports)
add_subdirectory(main)
add_subdirectory(music)
add_subdirectory(notifications)
add_subdirectory(renamer)
add_subdirectory(settings)

add_library(
  mediaelch_ui OBJECT
  movie_sets/MovieListDialog.cpp movie_sets/SetsWidget.cpp
  tv_show/TvShowFilesWidget.cpp media_centers/KodiSync.cpp
  support/SupportDialog.cpp
)

target_link_libraries(
  mediaelch_ui
  PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Sql
    Qt${QT_VERSION_MAJOR}::Network Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Multimedia
)
mediaelch_post_target_defaults(mediaelch_ui)
