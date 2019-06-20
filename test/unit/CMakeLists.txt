add_executable(mediaelch_unit main.cpp testCertification.cpp testImdbId.cpp testKodi.cpp movie/testMovieFileSearcher.cpp)

target_link_libraries(mediaelch_unit PRIVATE libmediaelch)

set_source_files_properties(main.cpp PROPERTIES COTIRE_EXCLUDED ON)
generate_coverage_report(mediaelch_unit)
catch_discover_tests(mediaelch_unit)
mediaelch_post_target_defaults(mediaelch_unit)

# Unit tests
add_custom_target(unit COMMAND $<TARGET_FILE:mediaelch_unit> --use-colour yes)
