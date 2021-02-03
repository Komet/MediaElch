#include "scrapers/movie/videobuster/VideoBusterSearchJob.h"

#include "scrapers/movie/videobuster/VideoBusterApi.h"

namespace mediaelch {
namespace scraper {

VideoBusterSearchJob::VideoBusterSearchJob(VideoBusterApi& api, MovieSearchJob::Config _config, QObject* parent) :
    MovieSearchJob(_config, parent), m_api{api}
{
}

void VideoBusterSearchJob::execute()
{
    m_api.searchForMovie(config().query, [this](QString data, ScraperError error) {
        if (error.hasError()) {
            m_error = error;

        } else {
            data = m_api.replaceEntities(data);
            parseSearch(data);
        }
        emit sigFinished(this);
    });
}

void VideoBusterSearchJob::parseSearch(const QString& html)
{
    int pos = 0;

    QRegExp rx("<div class=\"infos\"><a href=\"([^\"]*)\" class=\"title\">([^<]*)</a>");
    rx.setMinimal(true);

    while ((pos = rx.indexIn(html, pos)) != -1) {
        if (!rx.cap(1).isEmpty()) {
            MovieSearchJob::Result result;
            result.title = rx.cap(2);
            result.identifier = MovieIdentifier(rx.cap(1));
            m_results << result;
        }
        pos += rx.matchedLength();
    }
}

} // namespace scraper
} // namespace mediaelch
