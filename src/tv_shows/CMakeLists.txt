add_library(
  mediaelch_tvShows OBJECT
  model/EpisodeModelItem.cpp
  model/SeasonModelItem.cpp
  model/TvShowBaseModelItem.cpp
  model/TvShowModelItem.cpp
  model/TvShowRootModelItem.cpp
  EpisodeNumber.cpp
  EpisodeMap.cpp
  SeasonNumber.cpp
  SeasonOrder.cpp
  TvDbId.cpp
  TvMazeId.cpp
  TvShow.cpp
  TvShowEpisode.cpp
  TvShowFileSearcher.cpp
  TvShowModel.cpp
  TvShowProxyModel.cpp
  TvShowUpdater.cpp
  TvShowUtils.cpp
)

target_link_libraries(
  mediaelch_tvShows
  PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Sql
    Qt${QT_VERSION_MAJOR}::Network Qt${QT_VERSION_MAJOR}::Xml
    Qt${QT_VERSION_MAJOR}::MultimediaWidgets QuaZip::QuaZip
)
mediaelch_post_target_defaults(mediaelch_tvShows)
