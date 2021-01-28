add_library(libmediaelch)

add_subdirectory(cli)
add_subdirectory(concerts)
add_subdirectory(data)
add_subdirectory(export)
add_subdirectory(file)
add_subdirectory(globals)
add_subdirectory(image)
add_subdirectory(imports)
add_subdirectory(log)
add_subdirectory(media_centers)
add_subdirectory(movies)
add_subdirectory(network)
add_subdirectory(music)
add_subdirectory(renamer)
add_subdirectory(scrapers)
add_subdirectory(settings)
add_subdirectory(tv_shows)
add_subdirectory(ui)

target_link_libraries(
  libmediaelch
  PUBLIC
    Qt5::Concurrent
    Qt5::Core
    Qt5::Gui
    Qt5::Multimedia
    Qt5::MultimediaWidgets
    Qt5::Network
    Qt5::OpenGL
    Qt5::Qml
    Qt5::Quick
    Qt5::QuickWidgets
    Qt5::Sql
    Qt5::Widgets
    Qt5::Xml
  PRIVATE
    quazip5
    # MediaElch object libraries
    mediaelch_concert
    mediaelch_data
    mediaelch_downloads
    mediaelch_export
    mediaelch_file
    mediaelch_globals
    mediaelch_image
    mediaelch_image_providers
    mediaelch_log
    mediaelch_mediacenter
    mediaelch_movies
    mediaelch_music
    mediaelch_network
    mediaelch_renamer
    mediaelch_scrapers
    mediaelch_scraper_scraper_tmdb
    mediaelch_scraper_imdb_api
    mediaelch_scraper_movie_aebn
    mediaelch_scraper_movie_ade
    mediaelch_scraper_movie_custom
    mediaelch_scraper_movie_hotmovies
    mediaelch_scraper_movie_imdb
    mediaelch_scraper_movie_ofdb
    mediaelch_scraper_movie_tmdb
    mediaelch_scraper_movie_videobuster
    mediaelch_scraper_tv_custom
    mediaelch_scraper_tv_thetvdb
    mediaelch_scraper_tv_imdb
    mediaelch_scraper_tv_tmdb
    mediaelch_scraper_tv_tvmaze
    mediaelch_settings
    mediaelch_tvShows
    mediaelch_ui
    mediaelch_ui_smallwidgets
    mediaelch_ui_concerts
    mediaelch_ui_movies
    mediaelch_ui_shows
    mediaelch_ui_downloads
    mediaelch_ui_export
    mediaelch_ui_image
    mediaelch_ui_main
    mediaelch_ui_music
    mediaelch_ui_notifications
    mediaelch_ui_settings
)

# DL open/close
if(UNIX)
  target_link_libraries(libmediaelch PRIVATE curl)
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
endif()

# ------------------------------------------------------------------------------
# Sources
target_sources(libmediaelch PUBLIC ../data/MediaElch.qrc ../ui.qrc)

target_sources(libmediaelch PRIVATE qml/AlbumImageProvider.cpp)

mediaelch_post_target_defaults(libmediaelch)
