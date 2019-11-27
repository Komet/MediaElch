# Scraper tests: require an internet connection so they are not included in
# CTest
add_executable(
  mediaelch_test_scrapers
  main.cpp
  testAdultDvdEmpire.cpp
  testAEBN.cpp
  testHotMovies.cpp
  testIMDb.cpp
  testTMDb.cpp
  testTMDbConcerts.cpp
  testVideoBuster.cpp
  TheTvDb/testEpisodeLoader.cpp
  TheTvDb/testShowLoader.cpp
)

target_link_libraries(
  mediaelch_test_scrapers PRIVATE libmediaelch libmediaelch_mocks
                                  libmediaelch_testhelpers
)

mediaelch_post_target_defaults(mediaelch_test_scrapers)

# Scrapers are also not yet part of the coverage report
# generate_coverage_report(mediaelch_test_scrapers)

# Convenience target for executing all scraper tests;
add_custom_target(
  scraper_test COMMAND $<TARGET_FILE:mediaelch_test_scrapers> --use-colour yes
)
