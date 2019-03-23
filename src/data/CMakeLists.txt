add_library(
  mediaelch_data OBJECT
  # Sources
  Certification.cpp
  Database.cpp
  EpisodeNumber.cpp
  ImageCache.cpp
  ImdbId.cpp
  Rating.cpp
  SeasonNumber.cpp
  Storage.cpp
  StreamDetails.cpp
  Subtitle.cpp
  TmdbId.cpp
  TvDbId.cpp
  TvShow.cpp
  TvShowEpisode.cpp
  TvShowFileSearcher.cpp
)

target_link_libraries(
  mediaelch_data
  PRIVATE
    Qt5::Core
    Qt5::Concurrent
    Qt5::Widgets
    Qt5::Multimedia
    Qt5::Sql
    Qt5::Xml
)
mediaelch_post_target_defaults(mediaelch_data)
