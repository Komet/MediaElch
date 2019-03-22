#include "AdultDvdEmpire.h"

#include <QDebug>
#include <QRegExp>
#include <QTextDocument>

#include "data/Storage.h"
#include "globals/NetworkReplyWatcher.h"
#include "settings/Settings.h"

AdultDvdEmpire::AdultDvdEmpire(QObject* parent) :
    m_scraperSupports{MovieScraperInfos::Title,
        MovieScraperInfos::Released,
        MovieScraperInfos::Runtime,
        MovieScraperInfos::Overview,
        MovieScraperInfos::Poster,
        MovieScraperInfos::Actors,
        MovieScraperInfos::Genres,
        MovieScraperInfos::Studios,
        MovieScraperInfos::Backdrop,
        MovieScraperInfos::Set,
        MovieScraperInfos::Director}
{
    setParent(parent);
}

QString AdultDvdEmpire::name() const
{
    return QStringLiteral("Adult DVD Empire");
}

QString AdultDvdEmpire::identifier() const
{
    return scraperIdentifier;
}

bool AdultDvdEmpire::isAdult() const
{
    return true;
}

QVector<MovieScraperInfos> AdultDvdEmpire::scraperSupports()
{
    return m_scraperSupports;
}

QVector<MovieScraperInfos> AdultDvdEmpire::scraperNativelySupports()
{
    return m_scraperSupports;
}

std::vector<ScraperLanguage> AdultDvdEmpire::supportedLanguages()
{
    return {{tr("English"), "en"}};
}

void AdultDvdEmpire::changeLanguage(QString /*languageKey*/)
{
    // no-op: only one language is supported and hard-coded.
}

QString AdultDvdEmpire::defaultLanguageKey()
{
    return QStringLiteral("en");
}

QNetworkAccessManager* AdultDvdEmpire::qnam()
{
    return &m_qnam;
}

void AdultDvdEmpire::search(QString searchStr)
{
    QString encodedSearch = QUrl::toPercentEncoding(searchStr);
    QUrl url(QStringLiteral("https://www.adultdvdempire.com/dvd/search?view=list&q=%1").arg(encodedSearch));
    QNetworkReply* reply = qnam()->get(QNetworkRequest(url));
    new NetworkReplyWatcher(this, reply);
    connect(reply, &QNetworkReply::finished, this, &AdultDvdEmpire::onSearchFinished);
}

void AdultDvdEmpire::onSearchFinished()
{
    auto reply = static_cast<QNetworkReply*>(QObject::sender());
    reply->deleteLater();

    if (reply->error() != QNetworkReply::NoError) {
        qWarning() << "Network Error" << reply->errorString();
        emit searchDone(QVector<ScraperSearchResult>());
        return;
    }

    QString msg = QString::fromUtf8(reply->readAll());
    emit searchDone(parseSearch(msg));
}

QVector<ScraperSearchResult> AdultDvdEmpire::parseSearch(QString html)
{
    QTextDocument doc;
    QVector<ScraperSearchResult> results;
    int offset = 0;
    QRegExp rx(R"re(<a href="([^"]*)"[\n\t\s]*title="([^"]*)" Category="List Page" Label="Title">)re");
    rx.setMinimal(true);
    while ((offset = rx.indexIn(html, offset)) != -1) {
        doc.setHtml(rx.cap(2).trimmed());
        ScraperSearchResult result;
        result.id = rx.cap(1);
        result.name = doc.toPlainText();
        results << result;
        offset += rx.matchedLength();
    }

    return results;
}

void AdultDvdEmpire::loadData(QMap<MovieScraperInterface*, QString> ids, Movie* movie, QVector<MovieScraperInfos> infos)
{
    movie->clear(infos);
    QUrl url(QStringLiteral("https://www.adultdvdempire.com%1").arg(ids.values().first()));
    QNetworkReply* reply = qnam()->get(QNetworkRequest(url));
    new NetworkReplyWatcher(this, reply);
    reply->setProperty("storage", Storage::toVariant(reply, movie));
    reply->setProperty("infosToLoad", Storage::toVariant(reply, infos));
    reply->setProperty("id", ids.values().first());
    connect(reply, &QNetworkReply::finished, this, &AdultDvdEmpire::onLoadFinished);
}

void AdultDvdEmpire::onLoadFinished()
{
    auto reply = static_cast<QNetworkReply*>(QObject::sender());
    Movie* movie = reply->property("storage").value<Storage*>()->movie();
    reply->deleteLater();

    if (reply->error() == QNetworkReply::NoError) {
        QString msg = QString::fromUtf8(reply->readAll());
        parseAndAssignInfos(msg, movie, reply->property("infosToLoad").value<Storage*>()->movieInfosToLoad());
    } else {
        qWarning() << "Network Error" << reply->errorString();
    }

    movie->controller()->scraperLoadDone(this);
}

void AdultDvdEmpire::parseAndAssignInfos(QString html, Movie* movie, QVector<MovieScraperInfos> infos)
{
    QTextDocument doc;
    QRegExp rx;
    rx.setMinimal(true);

    rx.setPattern("<h1>(.*)</h1>");
    if (infos.contains(MovieScraperInfos::Title) && rx.indexIn(html) != -1) {
        doc.setHtml(rx.cap(1).trimmed());
        movie->setName(doc.toPlainText());
    }

    rx.setPattern("<small>Length: </small> ([0-9]*) hrs. ([0-9]*) mins.[\\s\\n]*</li>");
    if (infos.contains(MovieScraperInfos::Runtime) && rx.indexIn(html) != -1) {
        using namespace std::chrono;
        minutes runtime = hours(rx.cap(1).toInt()) + minutes(rx.cap(2).toInt());
        movie->setRuntime(runtime);
    }

    rx.setPattern("<li><small>Production Year:</small> ([0-9]{4})[\\s\\n]*</li>");
    if (infos.contains(MovieScraperInfos::Released) && rx.indexIn(html) != -1) {
        movie->setReleased(QDate::fromString(rx.cap(1), "yyyy"));
    }

    rx.setPattern("<li><small>Studio: </small><a href=\"[^\"]*\"[\\s\\n]*Category=\"Item Page\"[\\s\\n]*Label=\"Studio "
                  "- Details\">(.*)[\\s\\n]*</a>");
    if (infos.contains(MovieScraperInfos::Studios) && rx.indexIn(html) != -1) {
        doc.setHtml(rx.cap(1));
        movie->addStudio(doc.toPlainText().trimmed());
    }

    if (infos.contains(MovieScraperInfos::Actors)) {
        int offset = 0;
        rx.setPattern(
            R"re(<a href="/\d+/[^"]*"[\r\n\s]*Category="Item Page" Label="Performer">)re"
            R"re(<div class="[^"]+"><u>([^<]+)</u>.*<img src="([^"]+)")re");
        while ((offset = rx.indexIn(html, offset)) != -1) {
            offset += rx.matchedLength();
            Actor a;
            a.name = rx.cap(1);
            a.thumb = rx.cap(2);
            movie->addActor(a);
        }
    }

    rx.setPattern(R"(<a href="/\d+/[^"]+"\r\n\s+Category="Item Page" Label="Director">([^<]+)</a>)");
    if (infos.contains(MovieScraperInfos::Director) && rx.indexIn(html) != -1) {
        movie->setDirector(rx.cap(1).trimmed());
    }

    // get the list of categories first (to avoid parsing categories of other movies)
    rx.setPattern(R"(<strong>Categories:</strong>&nbsp;(.*)</div>)");
    if (infos.contains(MovieScraperInfos::Genres) && rx.indexIn(html) != -1) {
        QString categoryHtml = rx.cap(1);
        rx.setPattern(
            R"(<a href="[^"]*"[\r\s\n]*Category="Item Page" Label="Category">([^<]*)</a>)");
        int offset = 0;
        while ((offset = rx.indexIn(categoryHtml, offset)) != -1) {
            movie->addGenre(rx.cap(1).trimmed());
            offset += rx.matchedLength();
        }
    }

    rx.setPattern("<h4 class=\"m-b-0 text-dark synopsis\"><p>(.*)</p></h4>");
    if (infos.contains(MovieScraperInfos::Overview) && rx.indexIn(html) != -1) {
        doc.setHtml(rx.cap(1).trimmed());
        movie->setOverview(doc.toPlainText());
        if (Settings::instance()->usePlotForOutline()) {
            movie->setOutline(doc.toPlainText());
        }
    }

    rx.setPattern("href=\"([^\"]*)\"[\\s\\n]*id=\"front-cover\"");
    if (infos.contains(MovieScraperInfos::Poster) && rx.indexIn(html) != -1) {
        Poster p;
        p.thumbUrl = rx.cap(1);
        p.originalUrl = rx.cap(1);
        movie->images().addPoster(p);
    }

    rx.setPattern(
        R"(<a href="[^"]*"[\s\r\n]*Category="Item Page" Label="Series">[\s\r\n]*([^<]*)<span)");
    if (infos.contains(MovieScraperInfos::Set) && rx.indexIn(html) != -1) {
        doc.setHtml(rx.cap(1));
        QString set = doc.toPlainText().trimmed();
        if (set.endsWith("Series", Qt::CaseInsensitive)) {
            set.chop(6);
        }
        set = set.trimmed();
        if (set.startsWith("\"")) {
            set.remove(0, 1);
        }
        if (set.endsWith("\"")) {
            set.chop(1);
        }
        movie->setSet(set.trimmed());
    }

    if (infos.contains(MovieScraperInfos::Backdrop)) {
        rx.setPattern(R"re(<a rel="(scene)?screenshots"[\s\n]*href="([^"]*)")re");
        int offset = 0;
        while ((offset = rx.indexIn(html, offset)) != -1) {
            offset += rx.matchedLength();
            Poster p;
            p.thumbUrl = rx.cap(2);
            p.originalUrl = rx.cap(2);
            movie->images().addBackdrop(p);
        }
    }
}

bool AdultDvdEmpire::hasSettings() const
{
    return false;
}

void AdultDvdEmpire::loadSettings(const ScraperSettings& settings)
{
    Q_UNUSED(settings);
}

void AdultDvdEmpire::saveSettings(ScraperSettings& settings)
{
    Q_UNUSED(settings);
}

QWidget* AdultDvdEmpire::settingsWidget()
{
    return nullptr;
}
