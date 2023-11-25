#include "scrapers/movie/imdb/ImdbMovieSearchJob.h"

#include "scrapers/ScraperUtils.h"
#include "scrapers/imdb/ImdbApi.h"

#include <QFile>
#include <QRegularExpression>

namespace mediaelch {
namespace scraper {

ImdbMovieSearchJob::ImdbMovieSearchJob(ImdbApi& api, MovieSearchJob::Config _config, QObject* parent) :
    MovieSearchJob(_config, parent), m_api{api}
{
}

void ImdbMovieSearchJob::doStart()
{
    if (ImdbId::isValidFormat(config().query)) {
        searchViaImdbId();
    } else {
        searchViaQuery();
    }
}

void ImdbMovieSearchJob::searchViaImdbId()
{
    MediaElch_Debug_Ensures(ImdbId::isValidFormat(config().query));

    m_api.loadTitle(
        Locale("en"), ImdbId(config().query), ImdbApi::PageKind::Reference, [this](QString data, ScraperError error) {
            if (error.hasError()) {
                setScraperError(error);
            } else {
                parseIdFromMovieReferencePage(data);
            }
            emitFinished();
        });
}

void ImdbMovieSearchJob::searchViaQuery()
{
    MediaElch_Debug_Ensures(!ImdbId::isValidFormat(config().query));

    m_api.searchForMovie(Locale("en"), config().query, config().includeAdult, [this](QString data, ScraperError error) {
        if (error.hasError()) {
            setScraperError(error);
        } else {
            parseSearch(data);
        }
        emitFinished();
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
    // Search result table from "https://www.imdb.com/search/title/?title=..."
    static const QRegularExpression rx(R"(<a href="/title/(tt[\d]+)/[^>]+>(.+)</a>)",
        QRegularExpression::DotMatchesEverythingOption | QRegularExpression::InvertedGreedinessOption);
    // Entries are numbered: Remove Number.
    static const QRegularExpression listNo(
        R"(^\d+\.\s+)", QRegularExpression::DotMatchesEverythingOption | QRegularExpression::InvertedGreedinessOption);

    QRegularExpressionMatchIterator matches = rx.globalMatch(html);

    QRegularExpressionMatch match;
    while (matches.hasNext()) {
        match = matches.next();
        if (match.hasMatch()) {
            QString title = normalizeFromHtml(match.captured(2));
            title.remove(listNo);
            MovieSearchJob::Result result;
            result.title = title;
            result.identifier = MovieIdentifier(match.captured(1));
            // result.released = QDate::fromString(match.captured(3), "yyyy");
            m_results << result;
        }
    }
}

} // namespace scraper
} // namespace mediaelch
