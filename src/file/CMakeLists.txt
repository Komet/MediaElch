add_library(
  mediaelch_file OBJECT FileFilter.cpp NameFormatter.cpp FilenameUtils.cpp
                        Path.cpp
)

target_link_libraries(mediaelch_file PRIVATE Qt5::Core)
mediaelch_post_target_defaults(mediaelch_file)
