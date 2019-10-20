add_library(
  mediaelch_tvShows OBJECT
  EpisodeNumber.cpp
  model/EpisodeModelItem.cpp
  model/SeasonModelItem.cpp
  model/TvShowBaseModelItem.cpp
  model/TvShowModelItem.cpp
  model/TvShowRootModelItem.cpp
  SeasonNumber.cpp
  TvDbId.cpp
  TvShow.cpp
  TvShowEpisode.cpp
  TvShowFileSearcher.cpp
  TvShowModel.cpp
  TvShowProxyModel.cpp
  TvShowUpdater.cpp
)

target_link_libraries(
  mediaelch_tvShows PRIVATE Qt5::Widgets Qt5::Sql Qt5::Network Qt5::Xml
                            Qt5::MultimediaWidgets quazip5
)
mediaelch_post_target_defaults(mediaelch_tvShows)
