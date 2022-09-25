# Scraper tests: require an internet connection so they are not included in
# CTest
add_executable(
  mediaelch_test_scrapers
  imdbtv/testImdbTvEpisodeLoader.cpp
  imdbtv/testImdbTvHelper.cpp
  imdbtv/testImdbTvSeasonLoader.cpp
  imdbtv/testImdbTvShowLoader.cpp
  imdbtv/testImdbTvShowSearch.cpp
  main.cpp
  testAdultDvdEmpire.cpp
  testAEBN.cpp
  testHotMovies.cpp
  testImdbMovie.cpp
  testScraperHelpers.cpp
  testTmdbMovie.cpp
  testVideoBuster.cpp
  thetvdb/testTheTvDbEpisodeLoader.cpp
  thetvdb/testTheTvDbHelper.cpp
  thetvdb/testTheTvDbSearch.cpp
  thetvdb/testTheTvDbShowLoader.cpp
  tmdbconcert/testTmdbConcert.cpp
  tmdbtv/testTmdbApi.cpp
  tmdbtv/testTmdbTvEpisodeLoader.cpp
  tmdbtv/testTmdbTvHelper.cpp
  tmdbtv/testTmdbTvSeasonLoader.cpp
  tmdbtv/testTmdbTvShowLoader.cpp
  tmdbtv/testTmdbTvShowSearch.cpp
  tvmaze/testTvMazeEpisodeScrapeJob.cpp
  tvmaze/testTvMazeHelper.cpp
  tvmaze/testTvMazeSeasonScrapeJob.cpp
  tvmaze/testTvMazeShowScrapeJob.cpp
  tvmaze/testTvMazeShowSearch.cpp
  universalmusicscraper/testAllMusic.cpp
  universalmusicscraper/testUniversalMusicScraper.cpp
  universalmusicscraper/testUniversalMusicScraperHelper.cpp
)

target_link_libraries(
  mediaelch_test_scrapers PRIVATE libmediaelch libmediaelch_mocks
                                  libmediaelch_testhelpers
)

mediaelch_post_target_defaults(mediaelch_test_scrapers)

file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/test/resources)

# Scrapers are also not yet part of the coverage report
# generate_coverage_report(mediaelch_test_scrapers)

# Convenience target for executing all scraper tests;
# cmake-format: off
add_custom_target(
  scraper_test
  COMMAND
    $<TARGET_FILE:mediaelch_test_scrapers>
      --use-colour yes
      --resource-dir ${CMAKE_SOURCE_DIR}/test/resources
      --temp-dir ${CMAKE_BINARY_DIR}/test/resources
)
# cmake-format: on
