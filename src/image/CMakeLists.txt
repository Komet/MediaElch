add_library(
  mediaelch_image OBJECT Image.cpp ImageCapture.cpp ImageModel.cpp
                         ImageProxyModel.cpp ThumbnailDimensions.cpp
)

target_link_libraries(
  mediaelch_image
  PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Multimedia
          Qt${QT_VERSION_MAJOR}::Sql
)
mediaelch_post_target_defaults(mediaelch_image)
