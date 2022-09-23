add_library(
  mediaelch_file_search OBJECT
  TvShowFileSearcher.cpp
  MovieFilesOrganizer.cpp
  ConcertFileSearcher.cpp
  MusicFileSearcher.cpp
  movie/MovieDirectorySearcher.cpp
  movie/MovieFileSearcher.cpp
  movie/MovieDirScan.cpp
)

target_link_libraries(
  mediaelch_file_search
  PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    # TODO: Remove GUI once Globals.h does not depend on it anymore
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Network
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Sql
    Qt${QT_VERSION_MAJOR}::Concurrent
)
mediaelch_post_target_defaults(mediaelch_file_search)
