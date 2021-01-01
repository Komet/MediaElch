add_library(
  mediaelch_export OBJECT
  ExportTemplate.cpp ExportTemplateLoader.cpp MediaExport.cpp CsvExport.cpp
  SimpleEngine.cpp TableWriter.cpp
)

target_link_libraries(
  mediaelch_export PRIVATE Qt5::Core Qt5::Widgets Qt5::Network Qt5::Sql quazip5
)
mediaelch_post_target_defaults(mediaelch_export)
