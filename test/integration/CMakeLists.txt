add_executable(mediaelch_test_integration)

target_sources(
  mediaelch_test_integration
  PRIVATE
    export/testSimpleExport.cpp
    main.cpp
    file/testPath.cpp
    media_centers/testKodi_v18_concert.cpp
    media_centers/testKodi_v18_episode.cpp
    media_centers/testKodi_v18_movie.cpp
    media_centers/testKodi_v18_music_album.cpp
    media_centers/testKodi_v18_music_artist.cpp
    media_centers/testKodi_v18_show.cpp
    resource_dir.cpp
)

target_link_libraries(
  mediaelch_test_integration PRIVATE libmediaelch libmediaelch_testhelpers
)

mediaelch_post_target_defaults(mediaelch_test_integration)

file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/test/resources)

# cmake-format: off

# Discover all tests so that it can be used by CTest
catch_discover_tests(mediaelch_test_integration
  EXTRA_ARGS
    -r compact
    --resource-dir ${CMAKE_SOURCE_DIR}/test/resources
    --temp-dir ${CMAKE_BINARY_DIR}/test/resources
)

# Convenience target that is not used by CTest
add_custom_target(
  integration_test
  COMMAND
    $<TARGET_FILE:mediaelch_test_integration>
    --use-colour yes
    --resource-dir ${CMAKE_SOURCE_DIR}/test/resources
    --temp-dir ${CMAKE_BINARY_DIR}/test/resources
)
# cmake-format: on

if(ENABLE_COVERAGE)
  # We don't use the convenience generate_coverage_report function because the
  # executable requires a few more arguments.
  add_coverage_init()

  # cmake-format: off
  add_custom_target(coverage-integration COMMAND
      $<TARGET_FILE:mediaelch_test_integration>
      --use-colour yes
      --resource-dir ${CMAKE_SOURCE_DIR}/test/resources
      --temp-dir ${CMAKE_BINARY_DIR}/test/resources)
  # cmake-format: on

  add_dependencies(coverage-info coverage-integration)
  add_dependencies(coverage-integration coverage-init)
  add_dependencies(coverage coverage-integration)
endif()
