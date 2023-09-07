add_library(
  mediaelch_media OBJECT
  AsyncImage.cpp
  FileFilter.cpp
  FilenameUtils.cpp
  ImageCache.cpp
  ImageCapture.cpp
  ImageUtils.cpp
  MediaInfoFile.cpp
  NameFormatter.cpp
  Path.cpp
  StreamDetails.cpp
)

target_link_libraries(
  mediaelch_media
  PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Widgets
    # TODO: Remove GUI once Globals.h does not depend on it anymore
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Network
    Qt${QT_VERSION_MAJOR}::Concurrent
)
mediaelch_post_target_defaults(mediaelch_media)
