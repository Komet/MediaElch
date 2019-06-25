add_library(mediaelch_file OBJECT FileFilter.cpp)

target_link_libraries(mediaelch_file PRIVATE Qt5::Core)
mediaelch_post_target_defaults(mediaelch_file)
