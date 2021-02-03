#include "scrapers/movie/aebn/AebnSearchJob.h"

#include "scrapers/movie/aebn/AebnApi.h"

namespace mediaelch {
namespace scraper {

AebnSearchJob::AebnSearchJob(AebnApi& api, MovieSearchJob::Config _config, QString genre, QObject* parent) :
    MovieSearchJob(_config, parent), m_api{api}, m_genreId{std::move(genre)}
{
}

void AebnSearchJob::execute()
{
    m_api.searchForMovie(config().query, config().locale, m_genreId, [this](QString data, ScraperError error) {
        if (error.hasError()) {
            m_error = error;

        } else {
            parseSearch(data);
        }
        emit sigFinished(this);
    });
}

void AebnSearchJob::parseSearch(const QString& html)
{
    int offset = 0;
    QRegExp rx("<a id=\"FTSMovieSearch_link_image_detail_[0-9]+\" "
               "href=\"/dispatcher/"
               "movieDetail\\?genreId=([0-9]+)&amp;theaterId=([0-9]+)&amp;movieId=([0-9]+)([^\"]*)\" "
               "title=\"([^\"]*)\"><img src=\"([^\"]*)\" alt=\"([^\"]*)\" /></a>");
    rx.setMinimal(true);

    while ((offset = rx.indexIn(html, offset)) != -1) {
        if (!rx.cap(3).isEmpty()) {
            MovieSearchJob::Result result;
            result.identifier = MovieIdentifier(rx.cap(3));
            result.title = rx.cap(5);
            m_results << result;
        }
        offset += rx.matchedLength();
    }
}

} // namespace scraper
} // namespace mediaelch
