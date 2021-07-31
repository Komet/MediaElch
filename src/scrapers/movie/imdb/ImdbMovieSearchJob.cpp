#include "scrapers/movie/imdb/ImdbMovieSearchJob.h"

#include "scrapers/imdb/ImdbApi.h"

#include <QFile>
#include <QRegularExpression>

namespace mediaelch {
namespace scraper {

ImdbMovieSearchJob::ImdbMovieSearchJob(ImdbApi& api, MovieSearchJob::Config _config, QObject* parent) :
    MovieSearchJob(_config, parent), m_api{api}
{
}

void ImdbMovieSearchJob::start()
{
    if (ImdbId::isValidFormat(config().query)) {
        m_api.loadTitle(Locale("en"),
            ImdbId(config().query),
            ImdbApi::PageKind::Reference,
            [this](QString data, ScraperError error) {
                if (error.hasError()) {
                    m_error = error;
                } else {
                    parseIdFromMovieReferencePage(data);
                }
                emit sigFinished(this);
            });
        return;
    }

    m_api.searchForMovie(Locale("en"), config().query, config().includeAdult, [this](QString data, ScraperError error) {
        if (error.hasError()) {
            m_error = error;
        } else {
            parseSearch(data);
        }
        emit sigFinished(this);
    });
}

void ImdbMovieSearchJob::parseIdFromMovieReferencePage(const QString& html)
{
    MovieSearchJob::Result result;
    result.identifier = MovieIdentifier(config().query);

    QRegularExpression rx;
    rx.setPatternOptions(QRegularExpression::InvertedGreedinessOption);
    QRegularExpressionMatch match;

    rx.setPattern(R"re(<h3 itemprop="name">\n(.*)<span)re");
    match = rx.match(html);
    if (match.hasMatch()) {
        result.title = match.captured(1).trimmed();
    }

    // For search results, we are only interested in the year.
    rx.setPattern(R"re(<a href="/search/title\?year=(\d+)&)re");
    match = rx.match(html);
    if (match.hasMatch()) {
        result.released = QDate::fromString(match.captured(1), "yyyy");
    }

    if (!result.title.isEmpty()) {
        m_results << result;
    }
}

void ImdbMovieSearchJob::parseSearch(const QString& html)
{
    QRegularExpression rx;

    if (html.contains("Including Adult Titles")) {
        // Search result table from "https://www.imdb.com/search/title/?title=..."
        rx.setPattern(R"(<a href="/title/(tt[\d]+)/[^"]*"\n>([^<]*)</a>\n.*(?: \(I+\) |>)\(([0-9]*).*\))");
    } else {
        // Search result table from "https://www.imdb.com/find?q=..."
        rx.setPattern("<td class=\"result_text\"> <a href=\"/title/([t]*[\\d]+)/[^\"]*\" >([^<]*)</a>(?: \\(I+\\) | "
                      ")\\(([0-9]*)\\) (?:</td>|<br/>)");
    }

    rx.setPatternOptions(QRegularExpression::DotMatchesEverythingOption | QRegularExpression::InvertedGreedinessOption);
    QRegularExpressionMatchIterator matches = rx.globalMatch(html);

    while (matches.hasNext()) {
        QRegularExpressionMatch match = matches.next();
        if (!match.captured(1).isEmpty()) {
            MovieSearchJob::Result result;
            result.title = match.captured(2);
            result.identifier = MovieIdentifier(match.captured(1));
            result.released = QDate::fromString(match.captured(3), "yyyy");
            m_results << result;
        }
    }
}

} // namespace scraper
} // namespace mediaelch
