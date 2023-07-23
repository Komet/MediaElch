add_library(libmediaelch_testhelpers STATIC)
target_sources(
  libmediaelch_testhelpers
  PRIVATE
    matchers.cpp
    diff.cpp
    resource_dir.cpp
    reference_file.cpp
    normalize.cpp
    scraper_helpers.cpp
    fake_data.cpp
)

# Qt::Widgets due to MusicScraper.h
target_link_libraries(
  libmediaelch_testhelpers
  PRIVATE Qt${QT_VERSION_MAJOR}::Core Qt${QT_VERSION_MAJOR}::Xml
          Qt${QT_VERSION_MAJOR}::Network Qt${QT_VERSION_MAJOR}::Widgets
)
mediaelch_post_target_defaults(libmediaelch_testhelpers)
