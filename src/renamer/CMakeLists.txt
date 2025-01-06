add_library(
  mediaelch_renamer OBJECT MovieRenamer.cpp ConcertRenamer.cpp
                           EpisodeRenamer.cpp Renamer.cpp PlaceholderParser.cpp
)

target_link_libraries(
  mediaelch_renamer
  PRIVATE Qt${QT_VERSION_MAJOR}::Core Qt${QT_VERSION_MAJOR}::Multimedia
          Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Sql
)
mediaelch_post_target_defaults(mediaelch_renamer)
