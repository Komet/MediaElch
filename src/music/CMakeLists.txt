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
  mediaelch_music PRIVATE Qt5::Widgets Qt5::Sql Qt5::Network Qt5::Xml
                          Qt5::Concurrent
)
mediaelch_post_target_defaults(mediaelch_music)
