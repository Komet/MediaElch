add_library(
  mediaelch_model OBJECT
  MusicProxyModel.cpp
  ConcertModel.cpp
  ActorModel.cpp
  music/MusicModelRoles.cpp
  tv_show/TvShowModelItem.cpp
  tv_show/EpisodeModelItem.cpp
  tv_show/SeasonModelItem.cpp
  tv_show/TvShowBaseModelItem.cpp
  tv_show/TvShowUtils.cpp
  tv_show/TvShowRootModelItem.cpp
  ImageProxyModel.cpp
  TvShowModel.cpp
  MusicModel.cpp
  ImageModel.cpp
  MovieProxyModel.cpp
  MusicModelItem.cpp
  ConcertProxyModel.cpp
  MovieModel.cpp
  MediaStatusColumn.cpp
  RatingModel.cpp
  TvShowProxyModel.cpp
)

target_link_libraries(
  mediaelch_model
  PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    # TODO: Remove GUI and Widgets once Globals.h does not depend on it anymore
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Sql
    Qt${QT_VERSION_MAJOR}::Network
)
mediaelch_post_target_defaults(mediaelch_model)
