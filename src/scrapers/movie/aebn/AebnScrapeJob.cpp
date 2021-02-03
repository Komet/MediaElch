#include "scrapers/movie/aebn/AebnScrapeJob.h"

#include "movies/Movie.h"
#include "scrapers/movie/aebn/AebnApi.h"
#include "settings/Settings.h"

namespace mediaelch {
namespace scraper {

AebnScrapeJob::AebnScrapeJob(AebnApi& api, MovieScrapeJob::Config _config, QString genre, QObject* parent) :
    MovieScrapeJob(std::move(_config), parent), m_api{api}, m_genre{std::move(genre)}
{
}

void AebnScrapeJob::execute()
{
    m_api.loadMovie(config().identifier.str(),
        config().locale,
        m_genre,
        [this](QString data, ScraperError error) {
            if (!error.hasError()) {
                QStringList actorIds;
                parseAndAssignInfos(data, actorIds);

                if (config().details.contains(MovieScraperInfo::Actors) && !actorIds.isEmpty()) {
                    downloadActors(actorIds);
                    return;
                }
            } else {
                m_error = error;
            }
            emit sigFinished(this);
        });
}

void AebnScrapeJob::downloadActors(QStringList actorIds)
{
    if (actorIds.isEmpty()) {
        emit sigFinished(this);
        return;
    }

    QString id = actorIds.takeFirst();
    m_api.loadActor(id,
        config().locale,
        m_genre,
        [id, actorIds, this](QString data, ScraperError error) {
            if (!error.hasError()) {
                parseAndAssignActor(data, id);

            } else {
                m_error = error;
            }

            // Try to avoid a huge stack of nested lambdas.
            // With this we should return to the event loop and then execute this.
            // TODO: I'm not 100% sure that it works, though...
            QTimer::singleShot(0, [this, actorIds]() { downloadActors(actorIds); });
        });
}

void AebnScrapeJob::parseAndAssignInfos(const QString& html, QStringList& actorIds)
{
    QRegExp rx;
    rx.setMinimal(true);

    rx.setPattern(R"(<h1 itemprop="name"  class="md-movieTitle"  >(.*)</h1>)");
    if (rx.indexIn(html) != -1) {
        m_movie->setName(rx.cap(1));
    }

    rx.setPattern("<span class=\"runTime\"><span itemprop=\"duration\" content=\"([^\"]*)\">([0-9]+)</span>");
    if (rx.indexIn(html) != -1) {
        m_movie->setRuntime(std::chrono::minutes(rx.cap(2).toInt()));
    }

    rx.setPattern("<span class=\"detailsLink\" itemprop=\"datePublished\" content=\"([0-9]{4})(.*)\">");
    if (rx.indexIn(html) != -1) {
        m_movie->setReleased(QDate::fromString(rx.cap(1), "yyyy"));
    }

    rx.setPattern("<span itemprop=\"about\">(.*)</span>");
    if (rx.indexIn(html) != -1) {
        m_movie->setOverview(rx.cap(1));
    }

    rx.setPattern("<div id=\"md-boxCover\"><a href=\"([^\"]*)\" target=\"_blank\" onclick=\"([^\"]*)\"><img "
                  "itemprop=\"thumbnailUrl\" src=\"([^\"]*)\" alt=\"([^\"]*)\" name=\"boxImage\" id=\"boxImage\" "
                  "/></a>");
    if (rx.indexIn(html) != -1) {
        Poster p;
        p.thumbUrl = QString("https:") + rx.cap(3);
        p.originalUrl = QString("https:") + rx.cap(1);
        m_movie->images().addPoster(p);
    }

    rx.setPattern("<span class=\"detailsLink\"><a href=\"([^\"]*)\" class=\"series\">(.*)</a>");
    if (rx.indexIn(html) != -1) {
        MovieSet set;
        set.name = rx.cap(2);
        m_movie->setSet(set);
    }

    rx.setPattern("<span class=\"detailsLink\" itemprop=\"director\" itemscope "
                  "itemtype=\"http://schema.org/Person\">(.*)<a href=\"(.*)\" itemprop=\"name\">(.*)</a>");
    if (rx.indexIn(html) != -1) {
        m_movie->setDirector(rx.cap(3));
    }

    rx.setPattern("<a href=\"(.*)\" itemprop=\"productionCompany\">(.*)</a>");
    if (rx.indexIn(html) != -1) {
        m_movie->addStudio(rx.cap(2));
    }

    {
        int offset = 0;
        rx.setPattern("<a href=\"(.*)\"(.*) itemprop=\"genre\">(.*)</a>");
        while ((offset = rx.indexIn(html, offset)) != -1) {
            m_movie->addGenre(rx.cap(3));
            offset += rx.matchedLength();
        }
    }

    {
        int offset = 0;
        rx.setPattern("<a href=\"(.*)sexActs=[0-9]*(.*)\" (.*)>(.*)</a>");
        while ((offset = rx.indexIn(html, offset)) != -1) {
            m_movie->addTag(rx.cap(4));
            offset += rx.matchedLength();
        }
        offset = 0;
        rx.setPattern("<a href=\"(.*)positions=[0-9]*(.*)\" (.*)>(.*)</a>");
        while ((offset = rx.indexIn(html, offset)) != -1) {
            m_movie->addTag(rx.cap(4));
            offset += rx.matchedLength();
        }
    }

    {
        // clear actors
        m_movie->setActors({});

        int offset = 0;
        rx.setPattern("<a href=\"/dispatcher/starDetail\\?(.*)starId=([0-9]*)&amp;(.*)\"  class=\"linkWithPopup\" "
                      "onmouseover=\"(.*)\" onmouseout=\"killPopUp\\(\\)\"   itemprop=\"actor\" itemscope "
                      "itemtype=\"http://schema.org/Person\"><span itemprop=\"name\">(.*)</span></a>");
        while ((offset = rx.indexIn(html, offset)) != -1) {
            offset += rx.matchedLength();

            const QString actorName = rx.cap(5);
            const QVector<Actor*> actors = m_movie->actors();

            const bool actorAlreadyAdded = std::any_of(actors.cbegin(), actors.cend(), [&actorName](const Actor* a) { //
                return a->name == actorName;                                                                          //
            });

            if (actorAlreadyAdded) {
                continue;
            }

            Actor a;
            a.name = rx.cap(5);
            a.id = rx.cap(2);
            m_movie->addActor(a);
            if (Settings::instance()->downloadActorImages() && !actorIds.contains(rx.cap(2))) {
                actorIds.append(rx.cap(2));
            }
        }

        offset = 0;
        rx.setPattern("<a href=\"([^\"]*)\"   itemprop=\"actor\" itemscope itemtype=\"http://schema.org/Person\"><span "
                      "itemprop=\"name\">(.*)</span></a>");
        while ((offset = rx.indexIn(html, offset)) != -1) {
            offset += rx.matchedLength();

            const QString actorName = rx.cap(2);
            const QVector<Actor*> actors = m_movie->actors();

            const bool actorAlreadyAdded = std::any_of(actors.cbegin(), actors.cend(), [&actorName](const Actor* a) { //
                return a->name == actorName;                                                                          //
            });

            if (!actorAlreadyAdded) {
                Actor a;
                a.name = rx.cap(2);
                m_movie->addActor(a);
            }
        }
    }
}

void AebnScrapeJob::parseAndAssignActor(const QString& html, QString id)
{
    QRegExp rx(R"lit(<img itemprop="image" src="([^"]*)" alt="([^"]*)" class="star" />)lit");
    rx.setMinimal(true);
    if (rx.indexIn(html) != -1) {
        for (Actor* a : m_movie->actors()) {
            if (a->id == id) {
                a->thumb = QStringLiteral("https:") + rx.cap(1);
            }
        }
    }
}

} // namespace scraper
} // namespace mediaelch
