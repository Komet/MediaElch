#include "AEBN.h"

#include "data/Storage.h"
#include "log/Log.h"
#include "network/NetworkRequest.h"
#include "scrapers/movie/aebn/AebnSearchJob.h"
#include "ui/main/MainWindow.h"

#include <QGridLayout>
#include <QRegularExpression>

namespace mediaelch {
namespace scraper {

AEBN::AEBN(QObject* parent) :
    MovieScraper(parent),
    m_language{"en"},
    m_genreId{"101"}, // 101 => Straight
    m_widget{new QWidget(MainWindow::instance())},
    m_box{new QComboBox(m_widget)},
    m_genreBox{new QComboBox(m_widget)}
{
    m_meta.identifier = ID;
    m_meta.name = "AEBN";
    m_meta.description = "AEBN is a video database for adult content.";
    m_meta.website = "https://aebn.net";
    m_meta.termsOfService = "https://aebn.net";
    m_meta.privacyPolicy = "https://aebn.net";
    m_meta.help = "https://aebn.net";
    m_meta.supportedDetails = {MovieScraperInfo::Title,
        MovieScraperInfo::Released,
        MovieScraperInfo::Runtime,
        MovieScraperInfo::Overview,
        MovieScraperInfo::Poster,
        MovieScraperInfo::Actors,
        MovieScraperInfo::Genres,
        MovieScraperInfo::Studios,
        MovieScraperInfo::Director,
        MovieScraperInfo::Set,
        MovieScraperInfo::Tags};
    m_meta.supportedLanguages = {"bg",
        "zh",
        "hr",
        "cs",
        "da",
        "nl",
        "en",
        "fi",
        "fr",
        "de",
        "el",
        "he",
        "hu",
        "it",
        "ja",
        "ko",
        "no",
        "pl",
        "pt",
        "ru",
        "sl",
        "es",
        "sv",
        "tr"};
    m_meta.defaultLocale = "en";
    m_meta.isAdult = true;

    m_widget = new QWidget(MainWindow::instance());
    m_box = new QComboBox(m_widget);

    for (const mediaelch::Locale& lang : m_meta.supportedLanguages) {
        m_box->addItem(lang.languageTranslated(), lang.toString());
    }

    // Genre IDs overrides URL (http://[straight|gay]...)
    m_genreBox->addItem(tr("Straight"), "101");
    m_genreBox->addItem(tr("Gay"), "102");

    auto* layout = new QGridLayout(m_widget);
    layout->addWidget(new QLabel(tr("Language")), 0, 0);
    layout->addWidget(m_box, 0, 1);
    layout->addWidget(new QLabel(tr("Genre")), 1, 0);
    layout->addWidget(m_genreBox, 1, 1);
    layout->setColumnStretch(2, 1);
    layout->setContentsMargins(12, 0, 12, 12);
    m_widget->setLayout(layout);
}

const MovieScraper::ScraperMeta& AEBN::meta() const
{
    return m_meta;
}

void AEBN::initialize()
{
    // no-op
    // AEBN requires no initialization.
}

bool AEBN::isInitialized() const
{
    // AEBN requires no initialization.
    return true;
}

MovieSearchJob* AEBN::search(MovieSearchJob::Config config)
{
    return new AebnSearchJob(m_api, std::move(config), m_genreId, this);
}

void AEBN::changeLanguage(mediaelch::Locale locale)
{
    // Does not store the new language in settings.
    m_language = locale;
}

QSet<MovieScraperInfo> AEBN::scraperNativelySupports()
{
    return m_meta.supportedDetails;
}

void AEBN::loadData(QHash<MovieScraper*, mediaelch::scraper::MovieIdentifier> ids,
    Movie* movie,
    QSet<MovieScraperInfo> infos)
{
    if (ids.isEmpty()) {
        // TODO: Should not happen.
        return;
    }

    m_api.loadMovie(ids.constBegin().value().str(),
        m_language,
        m_genreId, //
        [movie, infos, this](QString data, ScraperError error) {
            movie->clear(infos);

            if (!error.hasError()) {
                QStringList actorIds;
                parseAndAssignInfos(data, movie, infos, actorIds);
                if (!actorIds.isEmpty()) {
                    downloadActors(movie, actorIds);
                    return;
                }
            } else {
                // TODO
                showNetworkError(error);
            }
            movie->controller()->scraperLoadDone(this, error);
        });
}

void AEBN::parseAndAssignInfos(QString html, Movie* movie, QSet<MovieScraperInfo> infos, QStringList& actorIds)
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

void AEBN::downloadActors(Movie* movie, QStringList actorIds)
{
    if (actorIds.isEmpty()) {
        movie->controller()->scraperLoadDone(this, {}); // done
        return;
    }

    QString id = actorIds.takeFirst();
    m_api.loadActor(id, m_language, m_genreId, [movie, id, actorIds, this](QString data, ScraperError error) {
        if (!error.hasError()) {
            parseAndAssignActor(data, movie, id);

        } else {
            // TODO
            showNetworkError(error);
        }

        // Try to avoid a huge stack of nested lambdas.
        // With this we should return to the event loop and then execute this.
        // TODO: I'm not 100% sure that it works, though...
        QTimer::singleShot(0, [this, movie, actorIds]() { downloadActors(movie, actorIds); });
    });
}

void AEBN::parseAndAssignActor(QString html, Movie* movie, QString id)
{
    QRegularExpression rx(R"lit(<img itemprop="image" src="([^"]*)" alt="([^"]*)" class="star" />)lit");
    rx.setPatternOptions(QRegularExpression::InvertedGreedinessOption | QRegularExpression::DotMatchesEverythingOption);

    QRegularExpressionMatch match = rx.match(html);
    if (match.hasMatch()) {
        for (Actor* a : movie->actors()) {
            if (a->id == id) {
                a->thumb = QStringLiteral("https:") + match.captured(1);
            }
        }
    }
}

bool AEBN::hasSettings() const
{
    return true;
}

void AEBN::loadSettings(ScraperSettings& settings)
{
    m_language = settings.language(m_meta.defaultLocale);
    for (int i = 0, n = m_box->count(); i < n; ++i) {
        if (m_box->itemData(i).toString() == m_language) {
            m_box->setCurrentIndex(i);
        }
    }
    m_genreId = settings.genre("101");
    for (int i = 0, n = m_genreBox->count(); i < n; ++i) {
        if (m_genreBox->itemData(i).toString() == m_genreId) {
            m_genreBox->setCurrentIndex(i);
        }
    }
}

void AEBN::saveSettings(ScraperSettings& settings)
{
    m_language = m_box->itemData(m_box->currentIndex()).toString();
    settings.setString("Language", m_language.toString());

    m_genreId = m_genreBox->itemData(m_genreBox->currentIndex()).toString();
    settings.setString("Genre", m_genreId);
}

QWidget* AEBN::settingsWidget()
{
    return m_widget;
}

} // namespace scraper
} // namespace mediaelch
