add_library(
  mediaelch_ui_settings OBJECT
  ConcertSettingsWidget.cpp
  ExportSettingsWidget.cpp
  ExportTemplateWidget.cpp
  GlobalSettingsWidget.cpp
  ImportSettingsWidget.cpp
  KodiSettingsWidget.cpp
  MovieSettingsWidget.cpp
  MusicSettingsWidget.cpp
  NetworkSettingsWidget.cpp
  ScraperSettingsWidget.cpp
  SettingsWindow.cpp
  TvShowSettingsWidget.cpp
  TvScraperSettingsWidget.cpp
  CustomTvScraperSettingsWidget.cpp
)

target_link_libraries(
  mediaelch_ui_settings PRIVATE Qt5::Widgets Qt5::Network Qt5::Sql Qt5::Xml
)
mediaelch_post_target_defaults(mediaelch_ui_settings)
