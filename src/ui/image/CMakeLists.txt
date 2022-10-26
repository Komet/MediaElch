add_library(
  mediaelch_ui_image OBJECT ImageDialog.cpp ImagePreviewDialog.cpp
                            ImageWidget.cpp
)

target_link_libraries(
  mediaelch_ui_image
  PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::QuickWidgets
          Qt${QT_VERSION_MAJOR}::Sql
)

if(Qt6_FOUND)
  target_link_libraries(mediaelch_ui_image PUBLIC Qt6::QuickControls2)
endif()

mediaelch_post_target_defaults(mediaelch_ui_image)
