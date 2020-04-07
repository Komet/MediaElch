#include "cli/info.h"

#include "Version.h"
#include "cli/common.h"
#include "export/TableWriter.h"
#include "globals/Manager.h"
#include "scrapers/movie/tmdb/TmdbMovie.h"

#include <QDebug>
#include <QEventLoop>
#include <iomanip>
#include <iostream>
#include <memory>

namespace mediaelch {
namespace cli {

static bool isScraperValid(const QString& scraper)
{
    static QSet<QString> knownScrapers = {"tmdb", "tvdb"};
    return knownScrapers.contains(scraper);
}

static bool isMovieScraper(const QString& scraper)
{
    static QSet<QString> knownMovieScrapers = {"tmdb"};
    return knownMovieScrapers.contains(scraper);
}

static bool isTvShowScraper(const QString& scraper)
{
    static QSet<QString> knownTvShowScrapers = {"tvdb"};
    return knownTvShowScrapers.contains(scraper);
}

static std::unique_ptr<scraper::MovieScraper> createScraper(const QString& scraper)
{
    if (scraper == "tmdb") {
        auto tmdb = std::make_unique<scraper::TmdbMovie>();

        // initialize
        QEventLoop loop;
        loop.connect(tmdb.get(), &scraper::MovieScraper::initialized, &loop, &QEventLoop::quit);
        tmdb->initialize();
        loop.exec();

        return tmdb;
    }
    throw std::runtime_error("Unhandled scraper id");
}

static void printMovieSearchResults(const QVector<scraper::MovieSearchJob::Result>& results)
{
    TableLayout layout;
    layout.addColumn(TableColumn("#", 3, ColumnAlignment::Right));
    layout.addColumn(TableColumn("Title", 55));
    layout.addColumn(TableColumn("Released", 10));
    layout.addColumn(TableColumn("Identifier", 15));

    TableWriter table(std::cout, layout);
    table.writeHeading();

    int i = 0;
    for (const auto& result : results) {
        table.writeCell(QString::number(i));
        table.writeCell(result.title);
        table.writeCell(result.released.toString("yyyy-MM-dd"));
        table.writeCell(result.identifier);
        ++i;
    }
}

static void printMovieInfo(Movie& movie)
{
    using namespace std::string_literals;

    TableLayout layout;
    layout.addColumn(TableColumn("Detail", 15));
    layout.addColumn(TableColumn("", 50));

    TableWriter table(std::cout, layout);
    table.writeHeading();

    table.writeCell("Name"s);
    table.writeCell(movie.name());

    table.writeCell("Original Name"s);
    table.writeCell(movie.originalName());

    table.writeCell("Released"s);
    table.writeCell(movie.released().toString("yyyy-MM-dd"));

    table.writeCell("Tagline"s);
    table.writeCell(movie.tagline());

    table.writeCell("Outline"s);
    table.writeCell(movie.outline());

    table.writeCell("Overview"s);
    table.writeCell(movie.overview());

    for (const auto& rating : movie.ratings()) {
        table.writeCell("Rating"s);
        table.writeCell(QStringLiteral("%1: %2 (%3 votes)")
                            .arg(rating.source, QString::number(rating.rating), QString::number(rating.voteCount)));
    }
}

static int handleMovieScraper(const QString& id, const QString& searchId, const QString loadId, QString lang)
{
    using namespace mediaelch::scraper;

    if (lang.isEmpty()) {
        lang = "en-US";
    }

    auto scraper = createScraper(id);

    if (!searchId.isEmpty()) {
        QEventLoop loop;
        bool isSuccessful = false;

        const auto handleSuccess = [&](QVector<MovieSearchJob::Result> res) { //
            isSuccessful = true;
            printMovieSearchResults(res);
            loop.quit();
        };
        const auto handleError = [&](ScraperSearchError error) {
            isSuccessful = false;
            std::cerr << "Error: " << error.message.toStdString();
            loop.quit();
        };

        MovieSearchJob::Config config(searchId, Locale(lang));
        auto* searchJob = scraper->search(config);

        scraper->connect(searchJob, &MovieSearchJob::sigSearchSuccess, handleSuccess);
        scraper->connect(searchJob, &MovieSearchJob::sigSearchError, handleError);
        loop.exec();

        return isSuccessful ? 0 : 1;
    }

    if (!loadId.isEmpty()) {
        QEventLoop loop;

        bool isSuccessful = false;
        Movie movie({});

        const auto handleSuccess = [&]() { //
            isSuccessful = true;
            printMovieInfo(movie);
            loop.quit();
        };
        const auto handleError = [&](ScraperLoadError error) {
            isSuccessful = false;
            std::cerr << "Error: " << error.message.toStdString();
            loop.quit();
        };

        MovieScrapeJob::Config config(loadId, Locale(lang), scraper->info().scraperSupports);
        auto* scrapeJob = scraper->scrape(movie, config);

        scraper->connect(scrapeJob, &MovieScrapeJob::sigScrapeSuccess, handleSuccess);
        scraper->connect(scrapeJob, &MovieScrapeJob::sigScrapeError, handleError);
        loop.exec();

        return isSuccessful ? 0 : 1;
    }

    return 1;
}

int scraper(QApplication& app, QCommandLineParser& parser)
{
    parser.clearPositionalArguments();
    // re-add this command so that it appears when help is printed
    parser.addPositionalArgument("scraper", "Test a scraper.", "scraper [list_options]");
    parser.addOption(QCommandLineOption("id", "Scraper ID.", "name"));
    parser.addOption(QCommandLineOption("search", "Search query. Cannot be combined with --load.", "query"));
    parser.addOption(
        QCommandLineOption("load", "Scraper query, e.g. IMDb id. Cannot be combined with --search.", "id"));
    parser.addOption(QCommandLineOption("lang", R"(Language code, e.g. "en-US", "pt-BR")", "iso-lang"));

    parser.process(app);

    const QStringList args = parser.positionalArguments();
    const QString command = args.size() < 2 ? QString() : args.at(1);

    if (!parser.isSet("id")) {
        std::cerr << "Missing scraper id!" << std::endl;
        return 1;
    }

    const QString id = parser.value("id");
    const QString searchId = parser.value("search");
    const QString loadId = parser.value("load");
    const QString language = parser.value("lang");

    if (!isScraperValid(id)) {
        std::cerr << "Unknown scraper id: " << parser.value("id").toStdString() << std::endl;
        return 1;
    }

    if (searchId.isEmpty() && loadId.isEmpty()) {
        std::cerr << "Missing --search or --load option!" << std::endl;
        return 1;
    }

    if (!searchId.isEmpty() && !loadId.isEmpty()) {
        std::cerr << "Cannot combine --search and --load!" << std::endl;
        return 1;
    }

    if (isMovieScraper(id)) {
        return handleMovieScraper(id, searchId, loadId, language);
    }

    if (isTvShowScraper(id)) {
        return 0;
    }

    qCritical() << "Unhandled scraper";

    return 1;
}

} // namespace cli
} // namespace mediaelch
