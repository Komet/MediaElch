add_library(
  mediaelch_globals OBJECT
  Globals.cpp
  MessageIds.cpp
  ScraperManager.cpp
  Manager.cpp
  Helper.cpp
  VersionInfo.cpp
  Module.cpp
)

target_link_libraries(
  mediaelch_globals
  PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Sql
    # TODO: Remove Widgets once Helpers.h does not depend on it anymore
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Network
    Qt${QT_VERSION_MAJOR}::Xml
)
mediaelch_post_target_defaults(mediaelch_globals)
