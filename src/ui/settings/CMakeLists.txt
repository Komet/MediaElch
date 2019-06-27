add_library(
  mediaelch_ui_settings
  OBJECT
  ExportTemplateWidget.cpp
  ExportSettingsWidget.cpp
  ImportSettingsWidget.cpp
  MovieSettingsWidget.cpp
  GlobalSettingsWidget.cpp
  ScraperSettingsWidget.cpp
  TvShowSettingsWidget.cpp
  SettingsWindow.cpp
)

if(APPLE)
  target_sources(mediaelch_ui_settings PRIVATE MacNotificationHandler.mm)
endif()

target_link_libraries(
  mediaelch_ui_settings
  PRIVATE Qt5::Widgets Qt5::Network Qt5::Sql Qt5::Xml
)
mediaelch_post_target_defaults(mediaelch_ui_settings)
