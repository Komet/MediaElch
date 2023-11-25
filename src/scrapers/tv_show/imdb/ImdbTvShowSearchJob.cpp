#include "scrapers/tv_show/imdb/ImdbTvShowSearchJob.h"

#include "data/tv_show/TvShow.h"
#include "scrapers/ScraperUtils.h"
#include "scrapers/tv_show/imdb/ImdbTvShowParser.h"

#include <QRegularExpression>

namespace mediaelch {
namespace scraper {

ImdbTvShowSearchJob::ImdbTvShowSearchJob(ImdbApi& api, ShowSearchJob::Config _config, QObject* parent) :
    ShowSearchJob(_config, parent), m_api{api}
{
}

void ImdbTvShowSearchJob::doStart()
{
    if (ImdbId::isValidFormat(config().query)) {
        searchViaImdbId();
    } else {
        searchViaQuery();
    }
}


void ImdbTvShowSearchJob::searchViaImdbId()
{
    MediaElch_Debug_Ensures(ImdbId::isValidFormat(config().query));

    ImdbId id = ImdbId(config().query);
    m_api.loadTitle(config().locale, id, ImdbApi::PageKind::Main, [this](QString html, ScraperError error) {
        if (!error.hasError()) {
            TvShow show;
            ImdbTvShowParser parser(show);
            error = parser.parseInfos(html);
            if (!error.hasError() && !show.title().isEmpty()) {
                ShowSearchJob::Result result;
                result.title = show.title();
                result.identifier = ShowIdentifier(config().query);
                result.released = show.firstAired();
                m_results.push_back(std::move(result));
            }
        }
        setScraperError(error);
        emitFinished();
    });
}

void ImdbTvShowSearchJob::searchViaQuery()
{
    MediaElch_Debug_Ensures(!ImdbId::isValidFormat(config().query));

    m_api.searchForShow(config().locale, config().query, [this](QString html, ScraperError error) {
        if (error.hasError()) {
            // pass; already set
        } else if (html.isEmpty()) {
            error.error = ScraperError::Type::NetworkError;
            error.message = tr("Loaded IMDb web page content is empty. Cannot scrape requested TV show.");

        } else if (is404(html)) {
            error.error = ScraperError::Type::InternalError;
            error.message = tr("Could not find result table in the scraped HTML. "
                               "Please contact MediaElch's developers.");

        } else {
            m_results = parseSearch(html);
        }
        setScraperError(error);
        emitFinished();
    });
}

QVector<ShowSearchJob::Result> ImdbTvShowSearchJob::parseSearch(const QString& html)
{
    // Note: Keep in sync with ImdbMovieSearchJob::parseSearch
    // TODO: De-duplicate?

    // Search result table from "https://www.imdb.com/search/title/?title=..."
    static const QRegularExpression rx(R"(<a href="/title/(tt[\d]+)/[^>]+>(.+)</a>.*(\d{4}))",
        QRegularExpression::DotMatchesEverythingOption | QRegularExpression::InvertedGreedinessOption);
    // Entries are numbered: Remove Number.
    static const QRegularExpression listNo(
        R"(^\d+\.\s+)", QRegularExpression::DotMatchesEverythingOption | QRegularExpression::InvertedGreedinessOption);

    QVector<ShowSearchJob::Result> results;
    QRegularExpressionMatchIterator matches = rx.globalMatch(html);
    QRegularExpressionMatch match;
    while (matches.hasNext()) {
        match = matches.next();
        if (match.hasMatch()) {
            QString title = normalizeFromHtml(match.captured(2));
            title.remove(listNo);
            ShowSearchJob::Result result;
            result.title = title;
            result.identifier = ShowIdentifier(match.captured(1));
            result.released = QDate::fromString(match.captured(3), "yyyy");
            results.push_back(std::move(result));
        }
    }

    return results;
}

QVector<ShowSearchJob::Result> ImdbTvShowSearchJob::parseResultFromShowPage(const QString& html)
{
    QRegularExpression rx(R"(<title>([^<]+?) \(TV Series (\d{4})–(\d{4}| )\) - IMDb</title>)");
    QRegularExpressionMatch match = rx.match(html);
    if (!match.hasMatch()) {
        return {};
    }

    ShowSearchJob::Result result;
    result.identifier = ShowIdentifier(config().query);
    result.title = match.captured(1);
    result.released = QDate::fromString(match.captured(2), "yyyy");

    return {result};
}

bool ImdbTvShowSearchJob::is404(const QString& html) const
{
    return QRegularExpression(R"(<title>404 Error)").match(html).hasMatch();
}


} // namespace scraper
} // namespace mediaelch
