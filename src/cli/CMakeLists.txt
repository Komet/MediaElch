add_executable(mediaelch_cli main.cpp)

target_link_libraries(mediaelch_cli PRIVATE libmediaelch)

target_sources(
  mediaelch_cli PRIVATE info.cpp list.cpp reload.cpp common.cpp show.cpp
                        info/ScraperFeatureTable.cpp
)

mediaelch_post_target_defaults(mediaelch_cli)
