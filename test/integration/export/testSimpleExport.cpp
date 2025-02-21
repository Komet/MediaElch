#include "test/test_helpers.h"

#include "export/SimpleEngine.h"
#include "test/helpers/resource_dir.h"

#include <memory>

using namespace mediaelch;

static QDir exportDir(QString subDir = "")
{
    return test::resourceDir().path() + "/export/" + subDir;
}

QVector<Movie*> fakeMovies()
{
    // not thread-safe
    static std::vector<std::unique_ptr<Movie>> movies;
    static QVector<Movie*> moviePtrs;
    if (movies.empty()) {
        movies.push_back(std::make_unique<Movie>());
        movies.push_back(std::make_unique<Movie>());
        movies.push_back(std::make_unique<Movie>());
        movies.push_back(std::make_unique<Movie>());

        movies[0]->setTitle("Spiderman - Coming Home");
        movies[1]->setTitle("Oceans 12");
        movies[2]->setTitle("Guardians of the Galaxy");
        movies[3]->setTitle("Jackass 3.5");

        moviePtrs.push_back(movies[0].get());
        moviePtrs.push_back(movies[1].get());
        moviePtrs.push_back(movies[2].get());
        moviePtrs.push_back(movies[3].get());
    }
    return moviePtrs;
}

TEST_CASE("Simple HTML export", "[export][simple]")
{
    ExportTemplate exportTemplate;
    exportTemplate.setName("Test Template");
    exportTemplate.setAuthors({"MediaElch authors"});
    exportTemplate.setTemplateEngine(ExportEngine::Simple);
    exportTemplate.setRemote(false);
    exportTemplate.setVersion("0.0.1");
    exportTemplate.setIdentifier("test-template");
    exportTemplate.addDescription("en", "Export Template for Testing");
    exportTemplate.setDirectory(mediaelch::DirectoryPath(exportDir("simple")));

    std::atomic_bool cancelFlag{false};

    SimpleEngine engine(exportTemplate, test::makeTempDir("export/simple"), cancelFlag);
    engine.exportMovies(fakeMovies());

    // just some basic checks
    // @todo better export tests
    QString moviesHtml = test::readTempFile("export/simple/movies.html");

    SECTION("contains list of movies")
    {
        CHECK(moviesHtml.contains("<a href=\"movies/3.html\">"));
        CHECK(moviesHtml.contains("Guardians of the Galaxy"));
        CHECK(moviesHtml.contains("() | Rating: n/a")); // empty year, no rating
    }

    SECTION("all tags replaced")
    {
        CHECK_FALSE(moviesHtml.contains("{"));
        CHECK_FALSE(moviesHtml.contains("}"));
    }
}
