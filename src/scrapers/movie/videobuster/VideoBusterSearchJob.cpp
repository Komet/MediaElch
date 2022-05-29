#include "scrapers/movie/videobuster/VideoBusterSearchJob.h"

#include "scrapers/movie/videobuster/VideoBusterApi.h"

#include <QRegularExpression>

namespace mediaelch {
namespace scraper {

VideoBusterSearchJob::VideoBusterSearchJob(VideoBusterApi& api, MovieSearchJob::Config _config, QObject* parent) :
    MovieSearchJob(_config, parent), m_api{api}
{
}

void VideoBusterSearchJob::start()
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
    QRegularExpression rx(
        R"re(<h3 class="name[^"]*"><a href="([^"]+)" title="[^"]+">([^<]+)<span class="[^"]+"> \((\d+)\)</span>)re");
    rx.setPatternOptions(QRegularExpression::InvertedGreedinessOption | QRegularExpression::DotMatchesEverythingOption);
    QRegularExpressionMatchIterator matches = rx.globalMatch(html);

    while (matches.hasNext()) {
        QRegularExpressionMatch match = matches.next();
        if (!match.captured(1).isEmpty()) {
            MovieSearchJob::Result result;
            result.title = match.captured(2);
            result.identifier = MovieIdentifier(match.captured(1));
            m_results << result;
        }
    }
}

} // namespace scraper
} // namespace mediaelch
