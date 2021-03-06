add_library(
  mediaelch_music OBJECT
  AllMusicId.cpp
  Album.cpp
  AlbumController.cpp
  Artist.cpp
  ArtistController.cpp
  MusicFileSearcher.cpp
  MusicModel.cpp
  MusicModelItem.cpp
  MusicProxyModel.cpp
  MusicBrainzId.cpp
  TheAudioDbId.cpp
)
target_link_libraries(
  mediaelch_music
  PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Sql
    Qt${QT_VERSION_MAJOR}::Network Qt${QT_VERSION_MAJOR}::Xml
    Qt${QT_VERSION_MAJOR}::Concurrent
)
mediaelch_post_target_defaults(mediaelch_music)
