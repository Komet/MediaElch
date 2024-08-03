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
  ScraperSettingsTable.cpp
  TvScraperInfoWidget.cpp
  MovieScraperInfoWidget.cpp
  CustomTvScraperSettingsWidget.cpp
  ConcertScraperInfoWidget.cpp
  MusicScraperInfoWidget.cpp
  ImageProviderInfoWidget.cpp
)

target_link_libraries(
  mediaelch_ui_settings
  PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Network
          Qt${QT_VERSION_MAJOR}::Sql Qt${QT_VERSION_MAJOR}::Xml
)
mediaelch_post_target_defaults(mediaelch_ui_settings)
