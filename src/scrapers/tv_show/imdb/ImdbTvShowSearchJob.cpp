#include "scrapers/tv_show/imdb/ImdbTvShowSearchJob.h"

#include <QRegularExpression>

namespace mediaelch {
namespace scraper {

ImdbTvShowSearchJob::ImdbTvShowSearchJob(ImdbApi& api, ShowSearchJob::Config _config, QObject* parent) :
    ShowSearchJob(_config, parent), m_api{api}
{
}

void ImdbTvShowSearchJob::execute()
{
    m_api.searchForShow(config().locale, config().query, [this](QString html, ScraperError error) {
        if (error.hasError()) {
            m_error = error;
        } else if (html.isEmpty()) {
            m_error.error = ScraperError::Type::NetworkError;
            m_error.message = tr("Loaded IMDb web page content is empty. Cannot scrape requested TV show.");

        } else if (is404(html)) {
            m_error.error = ScraperError::Type::InternalError;
            m_error.message = tr("Could not find result table in the scraped HTML. "
                                 "Please contact MediaElch's developers.");

        } else {
            m_results = parseSearch(html);
        }
        emit sigFinished(this);
    });
}

QVector<ShowSearchJob::Result> ImdbTvShowSearchJob::parseSearch(const QString& html)
{
    QRegularExpression rx(R"(<table class="findList">(.*?)</table>)", QRegularExpression::DotMatchesEverythingOption);
    QRegularExpressionMatch match = rx.match(html);
    if (!match.hasMatch()) {
        return parseResultFromShowPage(html);
    }

    QString searchTable = match.captured(1);
    rx.setPatternOptions(QRegularExpression::NoPatternOption);
    rx.setPattern(
        R"(<td class="result_text"> ?<a href="/title/tt(\d+)/[^"]*?" ?>([^<]*?)</a> \((\d{4})\) \(TV Series\) ?</td>)");

    QVector<ShowSearchJob::Result> results;
    QRegularExpressionMatch resultMatch = rx.match(searchTable, 0);
    while (resultMatch.hasMatch()) {
        ShowSearchJob::Result result;
        result.title = resultMatch.captured(2);
        result.released = QDate::fromString(resultMatch.captured(3), "yyyy");
        result.identifier = ShowIdentifier(QStringLiteral("tt") + resultMatch.captured(1));
        results.push_back(std::move(result));
        // Next result if it exists.
        resultMatch = rx.match(searchTable, resultMatch.capturedEnd());
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
