add_subdirectory(small_widgets)

add_library(mediaelch_ui STATIC tv_show/TvShowFilesWidget.cpp)

target_link_libraries(
  mediaelch_ui
  PRIVATE Qt5::Widgets Qt5::Sql Qt5::Network mediaelch_ui_smallwidgets
)
mediaelch_post_target_defaults(mediaelch_ui)
