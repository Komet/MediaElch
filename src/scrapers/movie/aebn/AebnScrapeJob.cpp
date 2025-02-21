#include "scrapers/movie/aebn/AebnScrapeJob.h"

#include "data/movie/Movie.h"
#include "scrapers/movie/aebn/AebnApi.h"
#include "settings/Settings.h"

#include <QRegularExpression>

namespace mediaelch {
namespace scraper {

AebnScrapeJob::AebnScrapeJob(AebnApi& api, MovieScrapeJob::Config _config, QString genre, QObject* parent) :
    MovieScrapeJob(std::move(_config), parent), m_api{api}, m_genre{std::move(genre)}
{
}

void AebnScrapeJob::doStart()
{
    m_api.loadMovie(config().identifier.str(), config().locale, m_genre, [this](QString data, ScraperError error) {
        if (!error.hasError()) {
            QStringList actorIds;
            parseAndAssignInfos(data, actorIds);

            if (config().details.contains(MovieScraperInfo::Actors) && !actorIds.isEmpty()) {
                downloadActors(actorIds);
                return;
            }
        } else {
            setScraperError(error);
        }
        emitFinished();
    });
}

void AebnScrapeJob::downloadActors(QStringList actorIds)
{
    if (actorIds.isEmpty()) {
        emitFinished();
        return;
    }

    QString id = actorIds.takeFirst();
    m_api.loadActor(id, config().locale, m_genre, [id, actorIds, this](QString data, ScraperError error) {
        if (!error.hasError()) {
            parseAndAssignActor(data, id);

        } else {
            setScraperError(error);
        }

        // Try to avoid a huge stack of nested lambdas.
        // With this we should return to the event loop and then execute this.
        // TODO: I'm not 100% sure that it works, though...
        QTimer::singleShot(0, [this, actorIds]() { downloadActors(actorIds); });
    });
}

void AebnScrapeJob::parseAndAssignInfos(const QString& html, QStringList& actorIds)
{
    QRegularExpression rx;
    rx.setPatternOptions(QRegularExpression::InvertedGreedinessOption | QRegularExpression::DotMatchesEverythingOption);
    QRegularExpressionMatch match;

    rx.setPattern(R"(<h1 itemprop="name"  class="md-movieTitle"  >(.*)</h1>)");
    match = rx.match(html);
    if (match.hasMatch()) {
        m_movie->setTitle(match.captured(1));
    }

    rx.setPattern("<span class=\"runTime\"><span itemprop=\"duration\" content=\"([^\"]*)\">([0-9]+)</span>");
    match = rx.match(html);
    if (match.hasMatch()) {
        m_movie->setRuntime(std::chrono::minutes(match.captured(2).toInt()));
    }

    rx.setPattern("<span class=\"detailsLink\" itemprop=\"datePublished\" content=\"([0-9]{4})(.*)\">");
    match = rx.match(html);
    if (match.hasMatch()) {
        m_movie->setReleased(QDate::fromString(match.captured(1), "yyyy"));
    }

    rx.setPattern("<span itemprop=\"about\">(.*)</span>");
    match = rx.match(html);
    if (match.hasMatch()) {
        m_movie->setOverview(match.captured(1));
    }

    rx.setPattern("<div id=\"md-boxCover\"><a href=\"([^\"]*)\" target=\"_blank\" onclick=\"([^\"]*)\"><img "
                  "itemprop=\"thumbnailUrl\" src=\"([^\"]*)\" alt=\"([^\"]*)\" name=\"boxImage\" id=\"boxImage\" "
                  "/></a>");
    match = rx.match(html);
    if (match.hasMatch()) {
        Poster p;
        p.thumbUrl = QString("https:") + match.captured(3);
        p.originalUrl = QString("https:") + match.captured(1);
        m_movie->images().addPoster(p);
    }

    rx.setPattern("<span class=\"detailsLink\"><a href=\"([^\"]*)\" class=\"series\">(.*)</a>");
    match = rx.match(html);
    if (match.hasMatch()) {
        MovieSet set;
        set.name = match.captured(2);
        m_movie->setSet(set);
    }

    rx.setPattern("<span class=\"detailsLink\" itemprop=\"director\" itemscope "
                  "itemtype=\"http://schema.org/Person\">(.*)<a href=\"(.*)\" itemprop=\"name\">(.*)</a>");
    match = rx.match(html);
    if (match.hasMatch()) {
        m_movie->setDirector(match.captured(3));
    }

    rx.setPattern("<a href=\"(.*)\" itemprop=\"productionCompany\">(.*)</a>");
    match = rx.match(html);
    if (match.hasMatch()) {
        m_movie->addStudio(match.captured(2));
    }

    {
        rx.setPattern("<a href=\"(.*)\"(.*) itemprop=\"genre\">(.*)</a>");
        QRegularExpressionMatchIterator matches = rx.globalMatch(html);
        while (matches.hasNext()) {
            m_movie->addGenre(matches.next().captured(3));
        }
    }

    {
        rx.setPattern("<a href=\"(.*)sexActs=[0-9]*(.*)\" (.*)>(.*)</a>");
        QRegularExpressionMatchIterator matches = rx.globalMatch(html);
        while (matches.hasNext()) {
            m_movie->addTag(matches.next().captured(4));
        }
        rx.setPattern("<a href=\"(.*)positions=[0-9]*(.*)\" (.*)>(.*)</a>");
        matches = rx.globalMatch(html);
        while (matches.hasNext()) {
            m_movie->addTag(matches.next().captured(4));
        }
    }

    {
        rx.setPattern("<a href=\"/dispatcher/starDetail\\?(.*)starId=([0-9]*)&amp;(.*)\"  class=\"linkWithPopup\" "
                      "onmouseover=\"(.*)\" onmouseout=\"killPopUp\\(\\)\"   itemprop=\"actor\" itemscope "
                      "itemtype=\"http://schema.org/Person\"><span itemprop=\"name\">(.*)</span></a>");
        QRegularExpressionMatchIterator matches = rx.globalMatch(html);
        while (matches.hasNext()) {
            match = matches.next();

            const QString actorName = match.captured(5);
            const auto& actors = m_movie->actors();

            const bool actorAlreadyAdded = std::any_of(actors.cbegin(), actors.cend(), [&actorName](const Actor* a) { //
                return a->name == actorName;                                                                          //
            });

            if (actorAlreadyAdded) {
                continue;
            }

            Actor a;
            a.name = match.captured(5);
            a.id = match.captured(2);
            m_movie->addActor(a);
            if (Settings::instance()->downloadActorImages() && !actorIds.contains(match.captured(2))) {
                actorIds.append(match.captured(2));
            }
        }

        rx.setPattern("<a href=\"([^\"]*)\"   itemprop=\"actor\" itemscope itemtype=\"http://schema.org/Person\"><span "
                      "itemprop=\"name\">(.*)</span></a>");
        matches = rx.globalMatch(html);
        while (matches.hasNext()) {
            match = matches.next();

            const QString actorName = match.captured(2);
            const auto& actors = m_movie->actors();

            const bool actorAlreadyAdded = std::any_of(actors.cbegin(), actors.cend(), [&actorName](const Actor* a) { //
                return a->name == actorName;                                                                          //
            });

            if (!actorAlreadyAdded) {
                Actor a;
                a.name = match.captured(2);
                m_movie->addActor(a);
            }
        }
    }
}

void AebnScrapeJob::parseAndAssignActor(const QString& html, QString id)
{
    QRegularExpression rx(R"lit(<img itemprop="image" src="([^"]*)" alt="([^"]*)" class="star" />)lit");
    rx.setPatternOptions(QRegularExpression::InvertedGreedinessOption | QRegularExpression::DotMatchesEverythingOption);

    QRegularExpressionMatch match = rx.match(html);
    if (match.hasMatch()) {
        for (Actor* a : m_movie->actors()) {
            if (a->id == id) {
                a->thumb = QStringLiteral("https:") + match.captured(1);
            }
        }
    }
}

} // namespace scraper
} // namespace mediaelch
