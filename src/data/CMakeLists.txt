add_library(
  mediaelch_data OBJECT
  Actor.cpp
  ActorModel.cpp
  Certification.cpp
  ImageCache.cpp
  ImdbId.cpp
  Locale.cpp
  MediaInfoFile.cpp
  MediaStatusColumn.cpp
  Rating.cpp
  RatingModel.cpp
  ResumeTime.cpp
  Storage.cpp
  StreamDetails.cpp
  Subtitle.cpp
  TmdbId.cpp
)

target_link_libraries(
  mediaelch_data
  PRIVATE Qt${QT_VERSION_MAJOR}::Core Qt${QT_VERSION_MAJOR}::Concurrent
          Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Multimedia
)
mediaelch_post_target_defaults(mediaelch_data)
