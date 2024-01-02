add_library(libmediaelch STATIC)

add_subdirectory(cli)
add_subdirectory(data)
add_subdirectory(database)
add_subdirectory(export)
add_subdirectory(file_search)
add_subdirectory(globals)
add_subdirectory(import)
add_subdirectory(log)
add_subdirectory(media)
add_subdirectory(media_center)
add_subdirectory(model)
add_subdirectory(network)
add_subdirectory(renamer)
add_subdirectory(scrapers)
add_subdirectory(settings)
add_subdirectory(ui)
add_subdirectory(utils)
add_subdirectory(workers)

target_link_libraries(
  libmediaelch
  PUBLIC
    Qt${QT_VERSION_MAJOR}::Concurrent
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Multimedia
    Qt${QT_VERSION_MAJOR}::MultimediaWidgets
    Qt${QT_VERSION_MAJOR}::Network
    Qt${QT_VERSION_MAJOR}::OpenGL
    Qt${QT_VERSION_MAJOR}::Svg
    Qt${QT_VERSION_MAJOR}::Sql
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Xml
  PRIVATE
    QuaZip::QuaZip
    # MediaElch object libraries
    mediaelch_data
    mediaelch_database
    mediaelch_export
    mediaelch_file_search
    mediaelch_globals
    mediaelch_image_providers
    mediaelch_import
    mediaelch_log
    mediaelch_media
    mediaelch_media_center
    mediaelch_model
    mediaelch_network
    mediaelch_renamer
    mediaelch_scraper_imdb_api
    mediaelch_scraper_movie_ade
    mediaelch_scraper_movie_aebn
    mediaelch_scraper_movie_custom
    mediaelch_scraper_movie_hotmovies
    mediaelch_scraper_movie_imdb
    mediaelch_scraper_movie_tmdb
    mediaelch_scraper_movie_videobuster
    mediaelch_scraper_scraper_tmdb
    mediaelch_scraper_tv_custom
    mediaelch_scraper_tv_fernsehserien
    mediaelch_scraper_tv_imdb
    mediaelch_scraper_tv_thetvdb
    mediaelch_scraper_tv_tmdb
    mediaelch_scraper_tv_tvmaze
    mediaelch_scrapers
    mediaelch_settings
    mediaelch_ui
    mediaelch_ui_concerts
    mediaelch_ui_export
    mediaelch_ui_image
    mediaelch_ui_import
    mediaelch_ui_main
    mediaelch_ui_movies
    mediaelch_ui_music
    mediaelch_ui_notifications
    mediaelch_ui_renamer
    mediaelch_ui_settings
    mediaelch_ui_small_widgets
    mediaelch_ui_tv_show
    mediaelch_utils
    mediaelch_workers
)

# DL open/close
if(UNIX)
  if(NOT APPLE)
    # libmediainfo requires dlopen/dlclose
    target_link_libraries(libmediaelch PRIVATE ${CMAKE_DL_LIBS})
  endif()
endif()

if(APPLE)
  find_library(FOUNDATION Foundation)
  mark_as_advanced(FOUNDATION)
  if(NOT FOUNDATION)
    message(FATAL_ERROR "Foundation framework not found")
  endif()
  target_link_libraries(libmediaelch PRIVATE "${FOUNDATION}")

  # Required for MacUiUtilities.mm for `effectiveAppearance`
  find_library(AppKit AppKit)
  mark_as_advanced(AppKit)
  if(NOT AppKit)
    message(FATAL_ERROR "AppKit framework not found")
  endif()

  target_link_libraries(libmediaelch PRIVATE "${AppKit}")
endif()

# ------------------------------------------------------------------------------
# Sources
target_sources(libmediaelch PUBLIC ../ui.qrc ../data/MediaElch.qrc)

mediaelch_post_target_defaults(libmediaelch)
