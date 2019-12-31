add_library(
  mediaelch_data OBJECT
  Certification.cpp
  Database.cpp
  ImageCache.cpp
  ImdbId.cpp
  MediaInfoFile.cpp
  Rating.cpp
  ResumeTime.cpp
  Storage.cpp
  StreamDetails.cpp
  Subtitle.cpp
  TmdbId.cpp
)

target_link_libraries(
  mediaelch_data PRIVATE Qt5::Core Qt5::Concurrent Qt5::Widgets Qt5::Multimedia
                         Qt5::Sql Qt5::Xml
)
mediaelch_post_target_defaults(mediaelch_data)
