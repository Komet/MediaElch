add_library(
  mediaelch_ui_renamer OBJECT RenamerDialog.cpp RenamerPlaceholdersWidget.cpp
)

target_link_libraries(
  mediaelch_ui_renamer
  PRIVATE Qt${QT_VERSION_MAJOR}::Core Qt${QT_VERSION_MAJOR}::Widgets
          Qt${QT_VERSION_MAJOR}::Network Qt${QT_VERSION_MAJOR}::Sql
)
mediaelch_post_target_defaults(mediaelch_ui_renamer)
