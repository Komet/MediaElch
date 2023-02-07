add_library(libmediaelch_testhelpers STATIC)
target_sources(
  libmediaelch_testhelpers
  PRIVATE matchers.cpp diff.cpp resource_dir.cpp reference_file.cpp
          normalize.cpp scraper_helpers.cpp
)
target_link_libraries(
  libmediaelch_testhelpers
  PRIVATE Qt${QT_VERSION_MAJOR}::Core Qt${QT_VERSION_MAJOR}::Xml
          Qt${QT_VERSION_MAJOR}::Network
)
mediaelch_post_target_defaults(libmediaelch_testhelpers)
