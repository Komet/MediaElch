add_library(
  mediaelch_image
  OBJECT
  Image.cpp
  ImageCapture.cpp
  ImageModel.cpp
  ImageProxyModel.cpp
  ImageWidget.cpp
)

target_link_libraries(
  mediaelch_image
  PRIVATE Qt5::Widgets Qt5::QuickWidgets Qt5::Sql
)
mediaelch_post_target_defaults(mediaelch_image)
