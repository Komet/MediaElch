add_executable(
  mediaelch-test
  main.cpp
  helpers/matchers.cpp
  data/testCertification.cpp
  data/testImdbId.cpp
  mocks/settings/MockScraperSettings.cpp
  scrapers/testAdultDvdEmpire.cpp
  scrapers/testAEBN.cpp
  scrapers/testHotMovies.cpp
  scrapers/testIMDb.cpp
  scrapers/testTMDb.cpp
  scrapers/testVideoBuster.cpp
  scrapers/testTMDbConcerts.cpp
)

target_link_libraries(mediaelch-test PRIVATE Qt5::Test libmediaelch)

# Cotire would use a PCH for Catch2 which results in `CATCH_CONFIG_RUNNER` not
# having any effect.
set_source_files_properties(main.cpp PROPERTIES COTIRE_EXCLUDED ON)

# If ENABLE_COVERAGE is set, generate a coverage report for this test.
generate_coverage_report(mediaelch-test)

# Let Catch2 find all tests so that CTest can list all tests instead of just one
# executable.
catch_discover_tests(mediaelch-test)

mediaelch_post_target_defaults(mediaelch-test)
