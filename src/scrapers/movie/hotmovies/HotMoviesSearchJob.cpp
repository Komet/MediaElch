#include "scrapers/movie/hotmovies/HotMoviesSearchJob.h"

#include "scrapers/movie/hotmovies/HotMoviesApi.h"

#include <QTextDocumentFragment>

namespace mediaelch {
namespace scraper {

HotMoviesSearchJob::HotMoviesSearchJob(HotMoviesApi& api, MovieSearchJob::Config _config, QObject* parent) :
    MovieSearchJob(_config, parent), m_api{api}
{
}

void HotMoviesSearchJob::execute()
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
    int offset = 0;

    QRegExp rx(R"lit(<div class="cell td_title">.*<h3 class="title">.*<a href="([^"]*)" title="[^"]*">(.*)</a>)lit");
    rx.setMinimal(true);

    while ((offset = rx.indexIn(html, offset)) != -1) {
        if (!rx.cap(1).isEmpty()) {
            MovieSearchJob::Result result;
            result.identifier = MovieIdentifier(rx.cap(1));
            result.title = QTextDocumentFragment::fromHtml(rx.cap(2)).toPlainText().trimmed();
            m_results << result;
        }
        offset += rx.matchedLength();
    }
}

} // namespace scraper
} // namespace mediaelch
