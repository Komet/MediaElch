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

void HotMoviesSearchJob::start()
{
    m_api.searchForMovie(config().query, [this](QString data, ScraperError error) {
        if (error.hasError()) {
            m_error = error;

        } else {
            parseSearch(data);
        }
        emit sigFinished(this);
    });
}

void HotMoviesSearchJob::parseSearch(const QString& html)
{
    QRegularExpression rx(
        R"lit(<div class="cell td_title">.*<h3 class="title">.*<a href="([^"]*)" title="[^"]*">(.*)</a>)lit");
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
