#include "scrapers/movie/adultdvdempire/AdultDvdEmpireSearchJob.h"

#include "scrapers/movie/adultdvdempire/AdultDvdEmpireApi.h"

#include <QTextDocument>

namespace mediaelch {
namespace scraper {

AdultDvdEmpireSearchJob::AdultDvdEmpireSearchJob(AdultDvdEmpireApi& api,
    MovieSearchJob::Config _config,
    QObject* parent) :
    MovieSearchJob(_config, parent), m_api{api}
{
}

void AdultDvdEmpireSearchJob::execute()
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


void AdultDvdEmpireSearchJob::parseSearch(const QString& html)
{
    QTextDocument doc;

    QRegularExpression rx(R"re(<a href="([^"]*)"[\n\t\s]*title="([^"]*)" Category="List Page" Label="Title">)re");
    rx.setPatternOptions(QRegularExpression::InvertedGreedinessOption);

    QRegularExpressionMatchIterator matches = rx.globalMatch(html);
    while (matches.hasNext()) {
        QRegularExpressionMatch match = matches.next();
        // DVDs vs VideoOnDemand (VOD)
        QString type;
        if (match.captured(1).endsWith("-movies.html")) {
            type = "[DVD] ";
        } else if (match.captured(1).endsWith("-blu-ray.html")) {
            type = "[BluRay] ";
        } else if (match.captured(1).endsWith("-videos.html")) {
            type = "[VOD] ";
        }

        if (!match.captured(1).isEmpty()) {
            doc.setHtml(match.captured(2).trimmed());
            MovieSearchJob::Result result;
            result.identifier = MovieIdentifier(match.captured(1));
            result.title = type + doc.toPlainText();
            m_results << result;
        }
    }
}

} // namespace scraper
} // namespace mediaelch
