add_library(
  mediaelch_ui_export OBJECT
  ExportDialog.cpp csv_export/CsvExportDialog.cpp
  csv_export/CsvExportModule.cpp csv_export/CsvExportConfiguration.h
  csv_export/CsvExportConfiguration.cpp
)

target_link_libraries(
  mediaelch_ui_export
  PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Network
          Qt${QT_VERSION_MAJOR}::Sql
)
mediaelch_post_target_defaults(mediaelch_ui_export)
