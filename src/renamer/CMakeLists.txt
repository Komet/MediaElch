add_library(
  mediaelch_renamer OBJECT
  ConcertRenamer.cpp EpisodeRenamer.cpp MovieRenamer.cpp Renamer.cpp
  RenamerDialog.cpp RenamerPlaceholders.cpp
)

target_link_libraries(
  mediaelch_renamer
  PRIVATE Qt${QT_VERSION_MAJOR}::Core Qt${QT_VERSION_MAJOR}::Widgets
          Qt${QT_VERSION_MAJOR}::Sql Qt${QT_VERSION_MAJOR}::Network
)
mediaelch_post_target_defaults(mediaelch_renamer)
