add_library(
  mediaelch_file OBJECT FileFilter.cpp NameFormatter.cpp FilenameUtils.cpp
                        Path.cpp Filesystem.cpp
)

target_link_libraries(mediaelch_file PRIVATE Qt${QT_VERSION_MAJOR}::Core)
mediaelch_post_target_defaults(mediaelch_file)
