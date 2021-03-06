add_library(mediaelch_log OBJECT Log.cpp)

# GUI is required due to Globals.h
target_link_libraries(
  mediaelch_log PRIVATE Qt${QT_VERSION_MAJOR}::Core
                        Qt${QT_VERSION_MAJOR}::Widgets
)
mediaelch_post_target_defaults(mediaelch_log)
