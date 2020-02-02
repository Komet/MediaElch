#include "AEBN.h"

#include <QDebug>
#include <QGridLayout>
#include <QRegExp>

#include "data/Storage.h"
#include "globals/NetworkReplyWatcher.h"
#include "ui/main/MainWindow.h"

AEBN::AEBN(QObject* parent) :
    m_scraperSupports{MovieScraperInfos::Title,
        MovieScraperInfos::Released,
        MovieScraperInfos::Runtime,
        MovieScraperInfos::Overview,
        MovieScraperInfos::Poster,
        MovieScraperInfos::Actors,
        MovieScraperInfos::Genres,
        MovieScraperInfos::Studios,
        MovieScraperInfos::Director,
        MovieScraperInfos::Set,
        MovieScraperInfos::Tags},
    m_language{"en"},
    m_genreId{"101"}, // 101 => Straight
    m_widget{new QWidget(MainWindow::instance())},
    m_box{new QComboBox(m_widget)},
    m_genreBox{new QComboBox(m_widget)}
{
    setParent(parent);

    m_widget = new QWidget(MainWindow::instance());
    m_box = new QComboBox(m_widget);

    for (const ScraperLanguage& lang : AEBN::supportedLanguages()) {
        m_box->addItem(lang.languageName, lang.languageKey);
    }

    // Genre IDs overrides URL (http://[straight|gay]...)
    m_genreBox->addItem(tr("Straight"), "101");
    m_genreBox->addItem(tr("Gay"), "102");

    auto layout = new QGridLayout(m_widget);
    layout->addWidget(new QLabel(QObject::tr("Language")), 0, 0);
    layout->addWidget(m_box, 0, 1);
    layout->addWidget(new QLabel(tr("Genre")), 1, 0);
    layout->addWidget(m_genreBox, 1, 1);
    layout->setColumnStretch(2, 1);
    layout->setContentsMargins(12, 0, 12, 12);
    m_widget->setLayout(layout);
}

std::vector<ScraperLanguage> AEBN::supportedLanguages()
{
    return {{tr("Bulgarian"), "bg"},
        {tr("Chinese"), "zh"},
        {tr("Croatian"), "hr"},
        {tr("Czech"), "cs"},
        {tr("Danish"), "da"},
        {tr("Dutch"), "nl"},
        {tr("English"), "en"},
        {tr("Finnish"), "fi"},
        {tr("French"), "fr"},
        {tr("German"), "de"},
        {tr("Greek"), "el"},
        {tr("Hebrew"), "he"},
        {tr("Hungarian"), "hu"},
        {tr("Italian"), "it"},
        {tr("Japanese"), "ja"},
        {tr("Korean"), "ko"},
        {tr("Norwegian"), "no"},
        {tr("Polish"), "pl"},
        {tr("Portuguese"), "pt"},
        {tr("Russian"), "ru"},
        {tr("Slovene"), "sl"},
        {tr("Spanish"), "es"},
        {tr("Swedish"), "sv"},
        {tr("Turkish"), "tr"}};
}

void AEBN::changeLanguage(QString languageKey)
{
    // Does not store the new language in settings.
    m_language = languageKey;
}

QString AEBN::defaultLanguageKey()
{
    return Settings::instance()->settings()->value("Language", "en").toString();
}

QString AEBN::name() const
{
    return QStringLiteral("AEBN");
}

QString AEBN::identifier() const
{
    return scraperIdentifier;
}

bool AEBN::isAdult() const
{
    return true;
}

QVector<MovieScraperInfos> AEBN::scraperSupports()
{
    return m_scraperSupports;
}

QVector<MovieScraperInfos> AEBN::scraperNativelySupports()
{
    return m_scraperSupports;
}

void AEBN::search(QString searchStr)
{
    QString encodedSearch = QUrl::toPercentEncoding(searchStr);
    QUrl url(QStringLiteral(
        "https://straight.theater.aebn.net/dispatcher/"
        "fts?userQuery=%2&targetSearchMode=basic&locale=%1&searchType=movie&sortType=Relevance&imageType="
        "Large&theaterId=822&genreId=%3")
                 .arg(m_language, encodedSearch, m_genreId));
    QNetworkReply* reply = m_qnam.get(QNetworkRequest(url));
    new NetworkReplyWatcher(this, reply);
    connect(reply, &QNetworkReply::finished, this, &AEBN::onSearchFinished);
}

void AEBN::onSearchFinished()
{
    auto* reply = dynamic_cast<QNetworkReply*>(QObject::sender());
    if (reply == nullptr) {
        qCritical() << "[AEBN] onSearchFinished: nullptr reply | Please report this issue!";
        emit searchDone({}, {ScraperSearchError::ErrorType::InternalError, tr("Internal Error: Please report!")});
        return;
    }
    reply->deleteLater();

    if (reply->error() != QNetworkReply::NoError) {
        qWarning() << "Network Error" << reply->errorString();
        emit searchDone({}, {ScraperSearchError::ErrorType::NetworkError, reply->errorString()});
        return;
    }

    QString msg = QString::fromUtf8(reply->readAll());
    emit searchDone(parseSearch(msg), {});
}

QVector<ScraperSearchResult> AEBN::parseSearch(QString html)
{
    QVector<ScraperSearchResult> results;
    int offset = 0;
    QRegExp rx("<a id=\"FTSMovieSearch_link_image_detail_[0-9]+\" "
               "href=\"/dispatcher/"
               "movieDetail\\?genreId=([0-9]+)&amp;theaterId=([0-9]+)&amp;movieId=([0-9]+)([^\"]*)\" "
               "title=\"([^\"]*)\"><img src=\"([^\"]*)\" alt=\"([^\"]*)\" /></a>");
    //    QRegExp rx("<a id=\"FTSMovieSearch_link_image_detail_[0-9]+\"
    //    href=\"/dispatcher/movieDetail\\?movieId=([0-9]+)([^\"]*)\" title=\"([^\"]*)\"><img src=\"([^\"]*)\"
    //    alt=\"([^\"]*)\" /></a>");
    rx.setMinimal(true);
    while ((offset = rx.indexIn(html, offset)) != -1) {
        ScraperSearchResult result;
        result.id = rx.cap(3);
        result.name = rx.cap(5);
        results << result;
        offset += rx.matchedLength();
    }

    return results;
}

void AEBN::loadData(QMap<MovieScraperInterface*, QString> ids, Movie* movie, QVector<MovieScraperInfos> infos)
{
    movie->clear(infos);

    QUrl url(QString(
        "https://straight.theater.aebn.net/dispatcher/movieDetail?movieId=%1&locale=%2&theaterId=822&genreId=%3")
                 .arg(ids.values().first(), m_language, m_genreId));
    QNetworkReply* reply = m_qnam.get(QNetworkRequest(url));
    new NetworkReplyWatcher(this, reply);
    reply->setProperty("storage", Storage::toVariant(reply, movie));
    reply->setProperty("infosToLoad", Storage::toVariant(reply, infos));
    connect(reply, &QNetworkReply::finished, this, &AEBN::onLoadFinished);
}

void AEBN::onLoadFinished()
{
    auto* reply = dynamic_cast<QNetworkReply*>(QObject::sender());
    Movie* movie = reply->property("storage").value<Storage*>()->movie();
    reply->deleteLater();

    if (reply->error() == QNetworkReply::NoError) {
        QString msg = QString::fromUtf8(reply->readAll());
        QStringList actorIds;
        parseAndAssignInfos(msg, movie, reply->property("infosToLoad").value<Storage*>()->movieInfosToLoad(), actorIds);
        if (!actorIds.isEmpty()) {
            downloadActors(movie, actorIds);
            return;
        }
    } else {
        showNetworkError(*reply);
        qWarning() << "Network Error" << reply->errorString();
    }
    movie->controller()->scraperLoadDone(this);
}

void AEBN::parseAndAssignInfos(QString html, Movie* movie, QVector<MovieScraperInfos> infos, QStringList& actorIds)
{
    QRegExp rx;
    rx.setMinimal(true);

    rx.setPattern(R"(<h1 itemprop="name"  class="md-movieTitle"  >(.*)</h1>)");
    if (infos.contains(MovieScraperInfos::Title) && rx.indexIn(html) != -1) {
        movie->setName(rx.cap(1));
    }

    rx.setPattern("<span class=\"runTime\"><span itemprop=\"duration\" content=\"([^\"]*)\">([0-9]+)</span>");
    if (infos.contains(MovieScraperInfos::Runtime) && rx.indexIn(html) != -1) {
        movie->setRuntime(std::chrono::minutes(rx.cap(2).toInt()));
    }

    rx.setPattern("<span class=\"detailsLink\" itemprop=\"datePublished\" content=\"([0-9]{4})(.*)\">");
    if (infos.contains(MovieScraperInfos::Released) && rx.indexIn(html) != -1) {
        movie->setReleased(QDate::fromString(rx.cap(1), "yyyy"));
    }

    rx.setPattern("<span itemprop=\"about\">(.*)</span>");
    if (infos.contains(MovieScraperInfos::Overview) && rx.indexIn(html) != -1) {
        movie->setOverview(rx.cap(1));
        if (Settings::instance()->usePlotForOutline()) {
            movie->setOutline(rx.cap(1));
        }
    }

    rx.setPattern("<div id=\"md-boxCover\"><a href=\"([^\"]*)\" target=\"_blank\" onclick=\"([^\"]*)\"><img "
                  "itemprop=\"thumbnailUrl\" src=\"([^\"]*)\" alt=\"([^\"]*)\" name=\"boxImage\" id=\"boxImage\" "
                  "/></a>");
    if (infos.contains(MovieScraperInfos::Poster) && rx.indexIn(html) != -1) {
        Poster p;
        p.thumbUrl = QString("https:") + rx.cap(3);
        p.originalUrl = QString("https:") + rx.cap(1);
        movie->images().addPoster(p);
    }

    rx.setPattern("<span class=\"detailsLink\"><a href=\"([^\"]*)\" class=\"series\">(.*)</a>");
    if (infos.contains(MovieScraperInfos::Set) && rx.indexIn(html) != -1) {
        MovieSet set;
        set.name = rx.cap(2);
        movie->setSet(set);
    }

    rx.setPattern("<span class=\"detailsLink\" itemprop=\"director\" itemscope "
                  "itemtype=\"http://schema.org/Person\">(.*)<a href=\"(.*)\" itemprop=\"name\">(.*)</a>");
    if (infos.contains(MovieScraperInfos::Director) && rx.indexIn(html) != -1) {
        movie->setDirector(rx.cap(3));
    }

    rx.setPattern("<a href=\"(.*)\" itemprop=\"productionCompany\">(.*)</a>");
    if (infos.contains(MovieScraperInfos::Studios) && rx.indexIn(html) != -1) {
        movie->addStudio(rx.cap(2));
    }

    if (infos.contains(MovieScraperInfos::Genres)) {
        int offset = 0;
        rx.setPattern("<a href=\"(.*)\"(.*) itemprop=\"genre\">(.*)</a>");
        while ((offset = rx.indexIn(html, offset)) != -1) {
            movie->addGenre(rx.cap(3));
            offset += rx.matchedLength();
        }
    }

    if (infos.contains(MovieScraperInfos::Tags)) {
        int offset = 0;
        rx.setPattern("<a href=\"(.*)sexActs=[0-9]*(.*)\" (.*)>(.*)</a>");
        while ((offset = rx.indexIn(html, offset)) != -1) {
            movie->addTag(rx.cap(4));
            offset += rx.matchedLength();
        }
        offset = 0;
        rx.setPattern("<a href=\"(.*)positions=[0-9]*(.*)\" (.*)>(.*)</a>");
        while ((offset = rx.indexIn(html, offset)) != -1) {
            movie->addTag(rx.cap(4));
            offset += rx.matchedLength();
        }
    }

    if (infos.contains(MovieScraperInfos::Actors)) {
        // clear actors
        movie->setActors({});

        int offset = 0;
        rx.setPattern("<a href=\"/dispatcher/starDetail\\?(.*)starId=([0-9]*)&amp;(.*)\"  class=\"linkWithPopup\" "
                      "onmouseover=\"(.*)\" onmouseout=\"killPopUp\\(\\)\"   itemprop=\"actor\" itemscope "
                      "itemtype=\"http://schema.org/Person\"><span itemprop=\"name\">(.*)</span></a>");
        while ((offset = rx.indexIn(html, offset)) != -1) {
            offset += rx.matchedLength();

            const bool actorAlreadyAdded =
                std::any_of(movie->actors().cbegin(), movie->actors().cend(), [&rx](const Actor* a) { //
                    return a->name == rx.cap(5);                                                      //
                });

            if (actorAlreadyAdded) {
                continue;
            }

            Actor a;
            a.name = rx.cap(5);
            a.id = rx.cap(2);
            movie->addActor(a);
            if (Settings::instance()->downloadActorImages() && !actorIds.contains(rx.cap(2))) {
                actorIds.append(rx.cap(2));
            }
        }

        offset = 0;
        rx.setPattern("<a href=\"([^\"]*)\"   itemprop=\"actor\" itemscope itemtype=\"http://schema.org/Person\"><span "
                      "itemprop=\"name\">(.*)</span></a>");
        while ((offset = rx.indexIn(html, offset)) != -1) {
            offset += rx.matchedLength();
            const bool actorAlreadyAdded =
                std::any_of(movie->actors().cbegin(), movie->actors().cend(), [&rx](const Actor* a) { //
                    return a->name == rx.cap(2);                                                      //
                });

            if (!actorAlreadyAdded) {
                Actor a;
                a.name = rx.cap(2);
                movie->addActor(a);
            }
        }
    }
}

void AEBN::downloadActors(Movie* movie, QStringList actorIds)
{
    if (actorIds.isEmpty()) {
        movie->controller()->scraperLoadDone(this);
        return;
    }

    QString id = actorIds.takeFirst();
    QUrl url(
        QString("https://straight.theater.aebn.net/dispatcher/starDetail?locale=%2&starId=%1&theaterId=822&genreId=%3")
            .arg(id, m_language, m_genreId));
    QNetworkReply* reply = m_qnam.get(QNetworkRequest(url));
    new NetworkReplyWatcher(this, reply);
    reply->setProperty("storage", Storage::toVariant(reply, movie));
    reply->setProperty("actorIds", actorIds);
    reply->setProperty("actorId", id);
    connect(reply, &QNetworkReply::finished, this, &AEBN::onActorLoadFinished);
}

void AEBN::onActorLoadFinished()
{
    auto* reply = dynamic_cast<QNetworkReply*>(QObject::sender());
    Movie* movie = reply->property("storage").value<Storage*>()->movie();
    QStringList actorIds = reply->property("actorIds").toStringList();
    QString actorId = reply->property("actorId").toString();
    reply->deleteLater();

    if (reply->error() == QNetworkReply::NoError) {
        QString msg = QString::fromUtf8(reply->readAll());
        parseAndAssignActor(msg, movie, actorId);
    } else {
        showNetworkError(*reply);
        qWarning() << "Network Error" << reply->errorString();
    }
    downloadActors(movie, actorIds);
}

void AEBN::parseAndAssignActor(QString html, Movie* movie, QString id)
{
    QRegExp rx(R"lit(<img itemprop="image" src="([^"]*)" alt="([^"]*)" class="star" />)lit");
    rx.setMinimal(true);
    if (rx.indexIn(html) != -1) {
        for (Actor* a : movie->actors()) {
            if (a->id == id) {
                a->thumb = QStringLiteral("https:") + rx.cap(1);
            }
        }
    }
}

bool AEBN::hasSettings() const
{
    return true;
}

void AEBN::loadSettings(const ScraperSettings& settings)
{
    m_language = settings.language();
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
    settings.setString("Language", m_language);

    m_genreId = m_genreBox->itemData(m_genreBox->currentIndex()).toString();
    settings.setString("Genre", m_genreId);
}

QWidget* AEBN::settingsWidget()
{
    return m_widget;
}
