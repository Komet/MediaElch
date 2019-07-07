add_executable(
  mediaelch_test_integration
  main.cpp
  resource_dir.cpp
  media_centers/testKodi_v16_movie.cpp
  media_centers/testKodi_v16_show.cpp
  media_centers/testKodi_v18_movie.cpp
  media_centers/testKodi_v18_show.cpp
)

target_link_libraries(
  mediaelch_test_integration
  PRIVATE libmediaelch libmediaelch_testhelpers
)

set_source_files_properties(main.cpp PROPERTIES COTIRE_EXCLUDED ON)
mediaelch_post_target_defaults(mediaelch_test_integration)

file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/test/resources)

add_custom_target(
  integration_test
  COMMAND
    $<TARGET_FILE:mediaelch_test_integration>
    --use-colour yes
    --resource-dir ${CMAKE_SOURCE_DIR}/test/resources
    --temp-dir ${CMAKE_BINARY_DIR}/test/resources
)

# Manual coverage setup. Requires that the coverage-init target has already been created.
add_custom_target(coverage-integration COMMAND
    $<TARGET_FILE:mediaelch_test_integration>
    --use-colour yes
    --resource-dir ${CMAKE_SOURCE_DIR}/test/resources
    --temp-dir ${CMAKE_BINARY_DIR}/test/resources)

add_dependencies(coverage-info coverage-integration)
add_dependencies(coverage-integration coverage-init)
add_dependencies(coverage coverage-integration)
