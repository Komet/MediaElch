add_library(mediaelch_ui_export OBJECT ExportDialog.cpp CsvExportDialog.cpp)

target_link_libraries(
  mediaelch_ui_export PRIVATE Qt5::Widgets Qt5::Network Qt5::Sql
)
mediaelch_post_target_defaults(mediaelch_ui_export)
