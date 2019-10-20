add_library(
  mediaelch_renamer OBJECT
  ConcertRenamer.cpp EpisodeRenamer.cpp MovieRenamer.cpp Renamer.cpp
  RenamerDialog.cpp RenamerPlaceholders.cpp
)

target_link_libraries(
  mediaelch_renamer PRIVATE Qt5::Core Qt5::Widgets Qt5::Sql Qt5::Network
)
mediaelch_post_target_defaults(mediaelch_renamer)
