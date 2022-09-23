add_library(
  mediaelch_media_center OBJECT
  KodiXml.cpp
  KodiVersion.cpp
  kodi/ArtistXmlReader.cpp
  kodi/ArtistXmlWriter.cpp
  kodi/KodiXmlWriter.cpp
  kodi/AlbumXmlReader.cpp
  kodi/EpisodeXmlReader.cpp
  kodi/MovieXmlReader.cpp
  kodi/MovieXmlWriter.cpp
  kodi/ConcertXmlWriter.cpp
  kodi/ConcertXmlReader.cpp
  kodi/EpisodeXmlWriter.cpp
  kodi/TvShowXmlReader.cpp
  kodi/AlbumXmlWriter.cpp
  kodi/TvShowXmlWriter.cpp
  MediaCenterInterface.cpp
)

target_link_libraries(
  mediaelch_media_center
  PRIVATE
    Qt${QT_VERSION_MAJOR}::Core Qt${QT_VERSION_MAJOR}::Multimedia
    Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Xml
    Qt${QT_VERSION_MAJOR}::Sql
)
mediaelch_post_target_defaults(mediaelch_media_center)
