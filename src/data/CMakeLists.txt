add_library(
  mediaelch_data OBJECT
  Actor.cpp
  ActorModel.cpp
  Certification.cpp
  Database.cpp
  ImageCache.cpp
  ImdbId.cpp
  Locale.cpp
  MediaInfoFile.cpp
  Rating.cpp
  ResumeTime.cpp
  Storage.cpp
  StreamDetails.cpp
  Subtitle.cpp
  TmdbId.cpp
)

target_link_libraries(
  mediaelch_data
  PRIVATE
    Qt${QT_VERSION_MAJOR}::Core Qt${QT_VERSION_MAJOR}::Concurrent
    Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Multimedia
    Qt${QT_VERSION_MAJOR}::Sql Qt${QT_VERSION_MAJOR}::Xml
)
mediaelch_post_target_defaults(mediaelch_data)
