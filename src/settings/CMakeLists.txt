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
  mediaelch_settings PRIVATE Qt5::Widgets Qt5::Network Qt5::Sql Qt5::Xml
                             mediaelch_data
)
mediaelch_post_target_defaults(mediaelch_settings)
