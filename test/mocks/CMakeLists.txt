add_library(
  libmediaelch_mocks STATIC settings/MockScraperSettings.cpp
                            settings/SettingsMock.cpp
)

target_link_libraries(
  libmediaelch_mocks
  PRIVATE Qt${QT_VERSION_MAJOR}::Core Qt${QT_VERSION_MAJOR}::Network
          Qt${QT_VERSION_MAJOR}::Gui Qt${QT_VERSION_MAJOR}::Widgets
)
mediaelch_post_target_defaults(libmediaelch_mocks)
