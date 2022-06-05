#include "scrapers/movie/hotmovies/HotMoviesSearchJob.h"

#include "scrapers/movie/hotmovies/HotMoviesApi.h"

#include <QRegularExpression>
#include <QTextDocumentFragment>

namespace mediaelch {
namespace scraper {

HotMoviesSearchJob::HotMoviesSearchJob(HotMoviesApi& api, MovieSearchJob::Config _config, QObject* parent) :
    MovieSearchJob(_config, parent), m_api{api}
{
}

void HotMoviesSearchJob::doStart()
{
    m_api.searchForMovie(config().query, [this](QString data, ScraperError error) {
        if (error.hasError()) {
            setScraperError(error);

        } else {
            parseSearch(data);
        }
        emitFinished();
    });
}

void HotMoviesSearchJob::parseSearch(const QString& html)
{
    QRegularExpression rx(R"re(<h3 class="title[^"]*">.*<a href="([^"]*)"[^>]*>(.*)</a>)re");
    rx.setPatternOptions(QRegularExpression::InvertedGreedinessOption | QRegularExpression::DotMatchesEverythingOption);

    QRegularExpressionMatchIterator matches = rx.globalMatch(html);
    while (matches.hasNext()) {
        QRegularExpressionMatch match = matches.next();
        if (!match.captured(1).isEmpty()) {
            MovieSearchJob::Result result;
            result.identifier = MovieIdentifier(match.captured(1));
            result.title = QTextDocumentFragment::fromHtml(match.captured(2)).toPlainText().trimmed();
            m_results << result;
        }
    }
}

} // namespace scraper
} // namespace mediaelch
