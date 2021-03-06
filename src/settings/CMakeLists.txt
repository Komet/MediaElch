add_library(
  mediaelch_settings OBJECT
  AdvancedSettings.cpp
  AdvancedSettingsXmlReader.cpp
  DataFile.cpp
  DirectorySettings.cpp
  ImportSettings.cpp
  KodiSettings.cpp
  NetworkSettings.cpp
  ScraperSettings.cpp
  Settings.cpp
)

target_link_libraries(
  mediaelch_settings
  PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Network
          Qt${QT_VERSION_MAJOR}::Sql Qt${QT_VERSION_MAJOR}::Xml mediaelch_data
)
mediaelch_post_target_defaults(mediaelch_settings)
