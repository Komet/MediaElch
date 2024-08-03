add_library(
  mediaelch_settings OBJECT
  ImportSettings.cpp
  DataFile.cpp
  AdvancedSettings.cpp
  DirectorySettings.cpp
  KodiSettings.cpp
  Settings.cpp
  AdvancedSettingsXmlReader.cpp
  NetworkSettings.cpp
)

target_link_libraries(
  mediaelch_settings
  PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    # TODO: Remove GUI and Widgets once Globals.h does not depend on it anymore
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Network
    Qt${QT_VERSION_MAJOR}::Sql
)
mediaelch_post_target_defaults(mediaelch_settings)
