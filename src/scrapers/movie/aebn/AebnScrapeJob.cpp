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
    // TODO: no-op
}

void AebnScrapeJob::downloadActors(QStringList actorIds)
{
    if (actorIds.isEmpty()) {
        emitFinished();
        return;
    }
    // TODO: no-op
}

void AebnScrapeJob::parseAndAssignInfos(const QString& html,
    QStringList& actorIds,
    Movie* movie,
    QSet<MovieScraperInfo> infos)
{
    QRegularExpression rx;
    rx.setPatternOptions(QRegularExpression::InvertedGreedinessOption | QRegularExpression::DotMatchesEverythingOption);
    QRegularExpressionMatch match;

    rx.setPattern(R"(<h1 itemprop="name"  class="md-movieTitle"  >(.*)</h1>)");
    match = rx.match(html);
    if (infos.contains(MovieScraperInfo::Title) && match.hasMatch()) {
        movie->setName(match.captured(1));
    }

    rx.setPattern("<span class=\"runTime\"><span itemprop=\"duration\" content=\"([^\"]*)\">([0-9]+)</span>");
    match = rx.match(html);
    if (infos.contains(MovieScraperInfo::Runtime) && match.hasMatch()) {
        movie->setRuntime(std::chrono::minutes(match.captured(2).toInt()));
    }

    rx.setPattern("<span class=\"detailsLink\" itemprop=\"datePublished\" content=\"([0-9]{4})(.*)\">");
    match = rx.match(html);
    if (infos.contains(MovieScraperInfo::Released) && match.hasMatch()) {
        movie->setReleased(QDate::fromString(match.captured(1), "yyyy"));
    }

    rx.setPattern("<span itemprop=\"about\">(.*)</span>");
    match = rx.match(html);
    if (infos.contains(MovieScraperInfo::Overview) && match.hasMatch()) {
        movie->setOverview(match.captured(1));
        if (Settings::instance()->usePlotForOutline()) {
            movie->setOutline(match.captured(1));
        }
    }

    rx.setPattern("<div id=\"md-boxCover\"><a href=\"([^\"]*)\" target=\"_blank\" onclick=\"([^\"]*)\"><img "
                  "itemprop=\"thumbnailUrl\" src=\"([^\"]*)\" alt=\"([^\"]*)\" name=\"boxImage\" id=\"boxImage\" "
                  "/></a>");
    match = rx.match(html);
    if (infos.contains(MovieScraperInfo::Poster) && match.hasMatch()) {
        Poster p;
        p.thumbUrl = QString("https:") + match.captured(3);
        p.originalUrl = QString("https:") + match.captured(1);
        movie->images().addPoster(p);
    }

    rx.setPattern("<span class=\"detailsLink\"><a href=\"([^\"]*)\" class=\"series\">(.*)</a>");
    match = rx.match(html);
    if (infos.contains(MovieScraperInfo::Set) && match.hasMatch()) {
        MovieSet set;
        set.name = match.captured(2);
        movie->setSet(set);
    }

    rx.setPattern("<span class=\"detailsLink\" itemprop=\"director\" itemscope "
                  "itemtype=\"http://schema.org/Person\">(.*)<a href=\"(.*)\" itemprop=\"name\">(.*)</a>");
    match = rx.match(html);
    if (infos.contains(MovieScraperInfo::Director) && match.hasMatch()) {
        movie->setDirector(match.captured(3));
    }

    rx.setPattern("<a href=\"(.*)\" itemprop=\"productionCompany\">(.*)</a>");
    match = rx.match(html);
    if (infos.contains(MovieScraperInfo::Studios) && match.hasMatch()) {
        movie->addStudio(match.captured(2));
    }

    if (infos.contains(MovieScraperInfo::Genres)) {
        rx.setPattern("<a href=\"(.*)\"(.*) itemprop=\"genre\">(.*)</a>");
        QRegularExpressionMatchIterator matches = rx.globalMatch(html);
        while (matches.hasNext()) {
            movie->addGenre(matches.next().captured(3));
        }
    }

    if (infos.contains(MovieScraperInfo::Tags)) {
        rx.setPattern("<a href=\"(.*)sexActs=[0-9]*(.*)\" (.*)>(.*)</a>");
        QRegularExpressionMatchIterator matches = rx.globalMatch(html);
        while (matches.hasNext()) {
            movie->addTag(matches.next().captured(4));
        }
        rx.setPattern("<a href=\"(.*)positions=[0-9]*(.*)\" (.*)>(.*)</a>");
        matches = rx.globalMatch(html);
        while (matches.hasNext()) {
            movie->addTag(matches.next().captured(4));
        }
    }

    if (infos.contains(MovieScraperInfo::Actors)) {
        // clear actors
        movie->setActors({});


        rx.setPattern("<a href=\"/dispatcher/starDetail\\?(.*)starId=([0-9]*)&amp;(.*)\"  class=\"linkWithPopup\" "
                      "onmouseover=\"(.*)\" onmouseout=\"killPopUp\\(\\)\"   itemprop=\"actor\" itemscope "
                      "itemtype=\"http://schema.org/Person\"><span itemprop=\"name\">(.*)</span></a>");
        QRegularExpressionMatchIterator matches = rx.globalMatch(html);
        while (matches.hasNext()) {
            match = matches.next();

            const QString actorName = match.captured(5);
            const auto& actors = movie->actors();

            const bool actorAlreadyAdded = std::any_of(actors.cbegin(), actors.cend(), [&actorName](const Actor* a) { //
                return a->name == actorName;                                                                          //
            });

            if (actorAlreadyAdded) {
                continue;
            }

            Actor a;
            a.name = match.captured(5);
            a.id = match.captured(2);
            movie->addActor(a);
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
            const auto& actors = movie->actors();

            const bool actorAlreadyAdded = std::any_of(actors.cbegin(), actors.cend(), [&actorName](const Actor* a) { //
                return a->name == actorName;                                                                          //
            });

            if (!actorAlreadyAdded) {
                Actor a;
                a.name = match.captured(2);
                movie->addActor(a);
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
