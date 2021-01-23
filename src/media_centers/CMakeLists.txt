add_library(
  mediaelch_mediacenter OBJECT
  kodi/KodiXmlWriter.cpp
  kodi/AlbumXmlReader.cpp
  kodi/AlbumXmlWriter.cpp
  kodi/ArtistXmlReader.cpp
  kodi/ArtistXmlWriter.cpp
  kodi/ConcertXmlReader.cpp
  kodi/ConcertXmlWriter.cpp
  kodi/EpisodeXmlReader.cpp
  kodi/EpisodeXmlWriter.cpp
  kodi/MovieXmlReader.cpp
  kodi/MovieXmlWriter.cpp
  kodi/TvShowXmlReader.cpp
  kodi/TvShowXmlWriter.cpp
  KodiVersion.cpp
  KodiXml.cpp
  MediaCenterInterface.cpp
)
target_link_libraries(
  mediaelch_mediacenter PRIVATE Qt5::Network Qt5::Widgets Qt5::Sql Qt5::Xml
)
mediaelch_post_target_defaults(mediaelch_mediacenter)
