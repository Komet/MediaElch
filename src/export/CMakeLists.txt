add_library(mediaelch_export OBJECT ExportTemplate.cpp ExportTemplateLoader.cpp)

target_link_libraries(
  mediaelch_export
  PRIVATE
    Qt5::Core
    Qt5::Widgets
    Qt5::Network
    Qt5::Sql
    quazip
)
mediaelch_post_target_defaults(mediaelch_export)
