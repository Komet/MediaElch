add_library(
  mediaelch_utils OBJECT Math.cpp Meta.cpp Random.cpp Containers.cpp Time.cpp
                         Optional.cpp
)

target_link_libraries(
  mediaelch_utils
  PRIVATE Qt${QT_VERSION_MAJOR}::Core Qt${QT_VERSION_MAJOR}::Gui
          Qt${QT_VERSION_MAJOR}::Network
)
mediaelch_post_target_defaults(mediaelch_utils)
