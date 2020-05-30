add_library(libmediaelch_mocks STATIC settings/MockScraperSettings.cpp)

target_link_libraries(libmediaelch_mocks PRIVATE Qt5::Core Qt5::Network)
mediaelch_post_target_defaults(libmediaelch_mocks)
