add_library(
  mediaelch_database OBJECT
  Database.cpp DatabaseId.cpp MoviePersistence.cpp ConcertPersistence.cpp
  MusicPersistence.cpp TvShowPersistence.cpp
)

target_link_libraries(
  mediaelch_database
  PUBLIC
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Sql
    # TODO: Remove GUI and Widgets once Globals.h does not depend on it anymore
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Network
    Qt${QT_VERSION_MAJOR}::Xml
)
mediaelch_post_target_defaults(mediaelch_database)
