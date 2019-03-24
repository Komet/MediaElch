add_executable(
  mediaelch_test_scrapers
  main.cpp
  testAdultDvdEmpire.cpp
  testAEBN.cpp
  testHotMovies.cpp
  testIMDb.cpp
  testTMDb.cpp
  testVideoBuster.cpp
  testTMDbConcerts.cpp
)

target_link_libraries(
  mediaelch_test_scrapers
  PRIVATE libmediaelch libmediaelch_mocks libmediaelch_testhelpers
)

set_source_files_properties(main.cpp PROPERTIES COTIRE_EXCLUDED ON)
generate_coverage_report(mediaelch_test_scrapers)
catch_discover_tests(mediaelch_test_scrapers)
mediaelch_post_target_defaults(mediaelch_test_scrapers)

# Unit tests
add_custom_target(
  scraper_test
  COMMAND $<TARGET_FILE:mediaelch_test_scrapers> --use-colour yes
)
