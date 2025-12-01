#include "scrapers/tv_show/imdb/ImdbTvShowSearchJob.h"

#include "data/tv_show/TvShow.h"
#include "scrapers/ScraperUtils.h"
#include "scrapers/tv_show/imdb/ImdbTvShowParser.h"

#include <QRegularExpression>

#include "scrapers/imdb/ImdbSearchPage.h"

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
    m_api.loadTitle(config().locale, id, ImdbApi::PageKind::Reference, [this](QString html, ScraperError error) {
        if (!error.hasError()) {
            TvShow show;
            ImdbTvShowParser parser(show, config().locale);
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
    auto results = ImdbSearchPage::parseSearch(html);
    QVector<ShowSearchJob::Result> showResults;
    for (const auto& result : results) {
        showResults << ShowSearchJob::Result{result.title, result.released, ShowIdentifier{result.identifier}};
    }
    return showResults;
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
