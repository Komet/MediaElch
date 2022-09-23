add_library(
  mediaelch_ui_image OBJECT ImageDialog.cpp ImagePreviewDialog.cpp
                            ImageWidget.cpp
)

target_link_libraries(
  mediaelch_ui_image
  PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::QuickWidgets
          Qt${QT_VERSION_MAJOR}::Sql
)
mediaelch_post_target_defaults(mediaelch_ui_image)
