add_executable(mediaelch_unit_test)

target_sources(
  mediaelch_unit_test
  PRIVATE
    main.cpp
    testModels.cpp
    data/testImdbId.cpp
    data/testLocale.cpp
    data/testTmdbId.cpp
    data/testCertification.cpp
    export/test.ExportTemplateLoader.cpp
    file/testNameFormatter.cpp
    file/testStackedBaseName.cpp
    globals/testVersionInfo.cpp
    globals/testTime.cpp
    movie/testMovieFileSearcher.cpp
    renamer/testPlaceholderParser.cpp
    scrapers/testImdbTvEpisodeParser.cpp
    scrapers/testImdbTvSeasonParser.cpp
    scrapers/custom_movie_scraper/StubMovieScraper.cpp
    scrapers/custom_movie_scraper/testCustomMovieScraper.cpp
    scrapers/testMovieMerger.cpp
    settings/testAdvancedSettings.cpp
    tv_shows/testEpisodeNumberExtraction.cpp
    tv_shows/testSeasonNumberExtraction.cpp
    tv_shows/testTvDbId.cpp
    tv_shows/testTvMazeId.cpp
)

target_link_libraries(
  mediaelch_unit_test PRIVATE libmediaelch libmediaelch_testhelpers
                              Qt${QT_VERSION_MAJOR}::Test
)

generate_coverage_report(mediaelch_unit_test)
catch_discover_tests(mediaelch_unit_test)
mediaelch_post_target_defaults(mediaelch_unit_test)

# Convenience unit test target with color output; does not use CTest
add_custom_target(
  unit_test COMMAND $<TARGET_FILE:mediaelch_unit_test> --use-colour yes
)
