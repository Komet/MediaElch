add_library(
  mediaelch_export OBJECT
  TableWriter.cpp CsvExport.cpp ExportTemplate.cpp SimpleEngine.cpp
  ExportTemplateLoader.cpp MediaExport.cpp
)

target_link_libraries(
  mediaelch_export
  PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    # TODO: Remove GUI once Globals.h does not depend on it anymore
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Network
    Qt${QT_VERSION_MAJOR}::Sql
    QuaZip::QuaZip
)

if(Qt6_FOUND)
  target_link_libraries(mediaelch_export PRIVATE Qt6::Core5Compat)
endif()

mediaelch_post_target_defaults(mediaelch_export)
