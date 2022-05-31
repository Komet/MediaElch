add_library(mediaelch_workers OBJECT Job.cpp)

target_link_libraries(
  mediaelch_workers PRIVATE Qt${QT_VERSION_MAJOR}::Core
                            Qt${QT_VERSION_MAJOR}::Concurrent
)
mediaelch_post_target_defaults(mediaelch_workers)
