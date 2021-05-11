#include "scrapers/movie/aebn/AebnSearchJob.h"

#include "scrapers/movie/aebn/AebnApi.h"

#include <QRegularExpression>

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
    QRegularExpression rx("<a id=\"FTSMovieSearch_link_image_detail_[0-9]+\" "
                          "href=\"/dispatcher/"
                          "movieDetail\\?genreId=([0-9]+)&amp;theaterId=([0-9]+)&amp;movieId=([0-9]+?)([^\"]*)\" "
                          "title=\"([^\"]*)\"><img src=\"([^\"]*)\" alt=\"([^\"]*)\" /></a>");
    //    QRegularExpression rx("<a id=\"FTSMovieSearch_link_image_detail_[0-9]+\"
    //    href=\"/dispatcher/movieDetail\\?movieId=([0-9]+)([^\"]*)\" title=\"([^\"]*)\"><img src=\"([^\"]*)\"
    //    alt=\"([^\"]*)\" /></a>");
    rx.setPatternOptions(QRegularExpression::InvertedGreedinessOption | QRegularExpression::DotMatchesEverythingOption);

    QRegularExpressionMatchIterator matches = rx.globalMatch(html);
    while (matches.hasNext()) {
        QRegularExpressionMatch match = matches.next();
        if (!match.captured(3).isEmpty()) {
            MovieSearchJob::Result result;
            result.identifier = MovieIdentifier(match.captured(3));
            result.title = match.captured(5);
            m_results << result;
        }
    }
}

} // namespace scraper
} // namespace mediaelch
