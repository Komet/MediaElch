add_subdirectory(movies)
add_subdirectory(concerts)
add_subdirectory(small_widgets)
add_subdirectory(tv_show)
add_subdirectory(downloads)
add_subdirectory(export)
add_subdirectory(image)
add_subdirectory(main)
add_subdirectory(music)
add_subdirectory(notifications)
add_subdirectory(settings)

add_library(
  mediaelch_ui OBJECT
  movie_sets/MovieListDialog.cpp movie_sets/SetsWidget.cpp
  tv_show/TvShowFilesWidget.cpp media_centers/KodiSync.cpp
  support/SupportDialog.cpp
)

target_link_libraries(
  mediaelch_ui PRIVATE Qt5::Widgets Qt5::Sql Qt5::Network Qt5::Core
                       Qt5::Multimedia
)
mediaelch_post_target_defaults(mediaelch_ui)
