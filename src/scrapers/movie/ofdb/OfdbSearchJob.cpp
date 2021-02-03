#include "scrapers/movie/ofdb/OfdbSearchJob.h"

#include "scrapers/movie/ofdb/OfdbApi.h"

#include <QDomDocument>

namespace mediaelch {
namespace scraper {

OfdbSearchJob::OfdbSearchJob(OfdbApi& api, MovieSearchJob::Config _config, QObject* parent) :
    MovieSearchJob(_config, parent), m_api{api}
{
}

void OfdbSearchJob::execute()
{
    const auto onDone = [this](QString data, ScraperError error) {
        if (error.hasError()) {
            m_error = error;
        } else {
            parseSearch(data);
        }

        emit sigFinished(this);
    };

    QRegularExpression rx("^id\\d+$"); // special handling if search string is an ID
    if (rx.match(config().query).hasMatch()) {
        m_api.loadMovie(config().query, onDone);
    } else {
        m_api.searchForMovie(config().query, onDone);
    }
}

void OfdbSearchJob::parseSearch(const QString& html)
{
    QDomDocument domDoc;
    domDoc.setContent(html);

    if (domDoc.elementsByTagName("eintrag").count() == 0 && !domDoc.elementsByTagName("resultat").isEmpty()) {
        QDomElement entry = domDoc.elementsByTagName("resultat").at(0).toElement();
        MovieSearchJob::Result result;
        result.identifier = MovieIdentifier(config().query.mid(2));
        if (entry.elementsByTagName("titel").size() > 0) {
            result.title = entry.elementsByTagName("titel").at(0).toElement().text();
        }
        if (entry.elementsByTagName("jahr").size() > 0) {
            result.released = QDate::fromString(entry.elementsByTagName("jahr").at(0).toElement().text(), "yyyy");
        }
        m_results << result;

    } else {
        for (int i = 0, n = domDoc.elementsByTagName("eintrag").size(); i < n; i++) {
            QDomElement entry = domDoc.elementsByTagName("eintrag").at(i).toElement();
            if (entry.elementsByTagName("id").size() == 0
                || entry.elementsByTagName("id").at(0).toElement().text().isEmpty()) {
                continue;
            }
            MovieSearchJob::Result result;
            result.identifier = MovieIdentifier(entry.elementsByTagName("id").at(0).toElement().text());
            if (entry.elementsByTagName("titel").size() > 0) {
                result.title = entry.elementsByTagName("titel").at(0).toElement().text();
            }
            if (entry.elementsByTagName("jahr").size() > 0) {
                result.released = QDate::fromString(entry.elementsByTagName("jahr").at(0).toElement().text(), "yyyy");
            }
            m_results << result;
        }
    }
}

} // namespace scraper
} // namespace mediaelch
