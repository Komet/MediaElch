add_library(libmediaelch_testhelpers STATIC)
target_sources(
  libmediaelch_testhelpers PRIVATE matchers.cpp xml_diff.cpp resource_dir.cpp
)
target_link_libraries(
  libmediaelch_testhelpers PRIVATE Qt${QT_VERSION_MAJOR}::Core
                                   Qt${QT_VERSION_MAJOR}::Xml
)
mediaelch_post_target_defaults(libmediaelch_testhelpers)
