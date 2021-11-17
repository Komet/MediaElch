add_library(
  mediaelch_export OBJECT
  ExportTemplate.cpp ExportTemplateLoader.cpp MediaExport.cpp CsvExport.cpp
  SimpleEngine.cpp TableWriter.cpp
)

target_link_libraries(
  mediaelch_export
  PRIVATE
    Qt${QT_VERSION_MAJOR}::Core Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Network Qt${QT_VERSION_MAJOR}::Sql QuaZip::QuaZip
)

if(Qt6_FOUND)
  target_link_libraries(mediaelch_export PRIVATE Qt6::Network Qt6::Core5Compat)
endif()

mediaelch_post_target_defaults(mediaelch_export)
