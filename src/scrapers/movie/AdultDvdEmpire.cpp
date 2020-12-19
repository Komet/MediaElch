#include "AdultDvdEmpire.h"

#include <QDebug>
#include <QRegExp>
#include <QTextDocument>

#include "data/Storage.h"
#include "network/NetworkRequest.h"
#include "settings/Settings.h"


namespace mediaelch {
namespace scraper {

AdultDvdEmpire::AdultDvdEmpire(QObject* parent) :
    m_scraperSupports{MovieScraperInfo::Title,
        MovieScraperInfo::Released,
        MovieScraperInfo::Runtime,
        MovieScraperInfo::Overview,
        MovieScraperInfo::Poster,
        MovieScraperInfo::Actors,
        MovieScraperInfo::Genres,
        MovieScraperInfo::Studios,
        MovieScraperInfo::Backdrop,
        MovieScraperInfo::Set,
        MovieScraperInfo::Director}
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

QSet<MovieScraperInfo> AdultDvdEmpire::scraperSupports()
{
    return m_scraperSupports;
}

QSet<MovieScraperInfo> AdultDvdEmpire::scraperNativelySupports()
{
    return m_scraperSupports;
}

QVector<mediaelch::Locale> AdultDvdEmpire::supportedLanguages()
{
    return {"en"};
}

void AdultDvdEmpire::changeLanguage(mediaelch::Locale /*locale*/)
{
    // no-op: only one language is supported and hard-coded.
}

mediaelch::Locale AdultDvdEmpire::defaultLanguage()
{
    return "en";
}

mediaelch::network::NetworkManager* AdultDvdEmpire::network()
{
    return &m_network;
}

void AdultDvdEmpire::search(QString searchStr)
{
    QString encodedSearch = QUrl::toPercentEncoding(searchStr);
    QUrl url(QStringLiteral("https://www.adultdvdempire.com/allsearch/search?view=list&q=%1").arg(encodedSearch));
    auto request = mediaelch::network::requestWithDefaults(url);
    QNetworkReply* reply = network()->getWithWatcher(request);
    connect(reply, &QNetworkReply::finished, this, &AdultDvdEmpire::onSearchFinished);
}

void AdultDvdEmpire::onSearchFinished()
{
    auto* reply = dynamic_cast<QNetworkReply*>(QObject::sender());
    if (reply == nullptr) {
        qCritical() << "[AdulDvdEmpire] onSearchFinished: nullptr reply | Please report this issue!";
        emit searchDone(
            {}, {ScraperError::Type::InternalError, tr("Internal Error: Please report!"), "nullptr dereference"});
        return;
    }
    reply->deleteLater();

    if (reply->error() != QNetworkReply::NoError) {
        qWarning() << "[AdultDvdEmpire] Search: Network Error" << reply->errorString();
        emit searchDone({}, mediaelch::replyToScraperError(*reply));
        return;
    }

    QString msg = QString::fromUtf8(reply->readAll());
    emit searchDone(parseSearch(msg), {});
}

QVector<ScraperSearchResult> AdultDvdEmpire::parseSearch(QString html)
{
    QTextDocument doc;
    QVector<ScraperSearchResult> results;
    int offset = 0;
    QRegExp rx(R"re(<a href="([^"]*)"[\n\t\s]*title="([^"]*)" Category="List Page" Label="Title">)re");
    rx.setMinimal(true);
    while ((offset = rx.indexIn(html, offset)) != -1) {
        // DVDs vs VideoOnDemand (VOD)
        QString type;
        if (rx.cap(1).endsWith("-movies.html")) {
            type = "[DVD] ";
        } else if (rx.cap(1).endsWith("-blu-ray.html")) {
            type = "[BluRay] ";
        } else if (rx.cap(1).endsWith("-videos.html")) {
            type = "[VOD] ";
        }
        doc.setHtml(rx.cap(2).trimmed());
        ScraperSearchResult result;
        result.id = rx.cap(1);
        result.name = type + doc.toPlainText();
        results << result;
        offset += rx.matchedLength();
    }

    return results;
}

void AdultDvdEmpire::loadData(QHash<MovieScraperInterface*, QString> ids, Movie* movie, QSet<MovieScraperInfo> infos)
{
    movie->clear(infos);
    QUrl url(QStringLiteral("https://www.adultdvdempire.com%1").arg(ids.values().first()));
    auto request = mediaelch::network::requestWithDefaults(url);
    mediaelch::network::useFirefoxUserAgent(request);
    QNetworkReply* reply = network()->getWithWatcher(request);
    reply->setProperty("storage", Storage::toVariant(reply, movie));
    reply->setProperty("infosToLoad", Storage::toVariant(reply, infos));
    reply->setProperty("id", ids.values().first());
    connect(reply, &QNetworkReply::finished, this, &AdultDvdEmpire::onLoadFinished);
}

void AdultDvdEmpire::onLoadFinished()
{
    auto* reply = dynamic_cast<QNetworkReply*>(QObject::sender());
    Movie* movie = reply->property("storage").value<Storage*>()->movie();
    reply->deleteLater();

    if (reply->error() == QNetworkReply::NoError) {
        QString msg = QString::fromUtf8(reply->readAll());
        parseAndAssignInfos(msg, movie, reply->property("infosToLoad").value<Storage*>()->movieInfosToLoad());

    } else {
        showNetworkError(*reply);
        qWarning() << "Network Error" << reply->errorString();
    }

    movie->controller()->scraperLoadDone(this);
}

void AdultDvdEmpire::parseAndAssignInfos(QString html, Movie* movie, QSet<MovieScraperInfo> infos)
{
    QTextDocument doc;
    QRegExp rx;
    rx.setMinimal(true);

    rx.setPattern("<h1>(.*)</h1>");
    if (infos.contains(MovieScraperInfo::Title) && rx.indexIn(html) != -1) {
        doc.setHtml(rx.cap(1).trimmed());
        movie->setName(doc.toPlainText());
    }

    rx.setPattern("<small>Length: </small> ([0-9]*) hrs. ([0-9]*) mins.[\\s\\n]*</li>");
    if (infos.contains(MovieScraperInfo::Runtime) && rx.indexIn(html) != -1) {
        using namespace std::chrono;
        minutes runtime = hours(rx.cap(1).toInt()) + minutes(rx.cap(2).toInt());
        movie->setRuntime(runtime);
    }

    rx.setPattern("<li><small>Production Year:</small> ([0-9]{4})[\\s\\n]*</li>");
    if (infos.contains(MovieScraperInfo::Released) && rx.indexIn(html) != -1) {
        movie->setReleased(QDate::fromString(rx.cap(1), "yyyy"));
    }

    rx.setPattern("<li><small>Studio: </small><a href=\"[^\"]*\"[\\s\\n]*Category=\"Item Page\"[\\s\\n]*Label=\"Studio "
                  "- Details\">(.*)[\\s\\n]*</a>");
    if (infos.contains(MovieScraperInfo::Studios) && rx.indexIn(html) != -1) {
        doc.setHtml(rx.cap(1));
        movie->addStudio(doc.toPlainText().trimmed());
    }

    if (infos.contains(MovieScraperInfo::Actors)) {
        // clear actors
        movie->setActors({});

        QTextDocument text;

        // qInfo() << html;

        int offset = 0;
        // The Regex is "a bit" more complex because ADE has two HTML styles:
        // One with images and one without. The second Regex line has an OR for this.
        rx.setPattern(
            R"re(<a href="/\d+/[^"]+"\r?\n\s+style="[^"]+"\r?\n\s+Category="Item Page" Label="Performer">)re"
            R"re((?:(?:<div class="[^"]+"><u>([^<]+)</u>(?:<div[^>]+>)*<img src="([^"]+)")|(?:(?:\r?\n\t+)+(.+)</a>)))re");
        while ((offset = rx.indexIn(html, offset)) != -1) {
            offset += rx.matchedLength();
            Actor a;
            if (rx.cap(1).isEmpty()) {
                text.setHtml(rx.cap(3).trimmed());
                a.name = text.toPlainText();
            } else {
                text.setHtml(rx.cap(1).trimmed());
                a.name = text.toPlainText();
                a.thumb = rx.cap(2);
            }
            movie->addActor(a);
        }
    }

    rx.setPattern(R"(<a href="/\d+/[^"]+"\r\n\s+Category="Item Page" Label="Director">([^<]+)</a>)");
    if (infos.contains(MovieScraperInfo::Director) && rx.indexIn(html) != -1) {
        movie->setDirector(rx.cap(1).trimmed());
    }

    // get the list of categories first (to avoid parsing categories of other movies)
    rx.setPattern(R"(<strong>Categories:</strong>&nbsp;(.*)</div>)");
    if (infos.contains(MovieScraperInfo::Genres) && rx.indexIn(html) != -1) {
        QString categoryHtml = rx.cap(1);
        rx.setPattern(R"(<a href="[^"]*"[\r\s\n]*Category="Item Page" Label="Category">([^<]*)</a>)");
        int offset = 0;
        while ((offset = rx.indexIn(categoryHtml, offset)) != -1) {
            movie->addGenre(rx.cap(1).trimmed());
            offset += rx.matchedLength();
        }
    }

    rx.setPattern(R"(<h4 class="m-b-0 text-dark synopsis">(<p( class="markdown-h[12]")?>.*)</p></h4>)");
    if (infos.contains(MovieScraperInfo::Overview) && rx.indexIn(html) != -1) {
        // add some newlines to simulate the paragraphs (scene descriptions)
        QString content{rx.cap(1).trimmed()};
        content.remove("<p class=\"markdown-h1\">");
        content.remove("<p>");
        content.replace("<p class=\"markdown-h2\">", "<br>");
        content.replace("</p>", "<br>");
        doc.setHtml(content);
        movie->setOverview(doc.toPlainText());
        if (Settings::instance()->usePlotForOutline()) {
            movie->setOutline(doc.toPlainText());
        }
    }

    rx.setPattern("href=\"([^\"]*)\"[\\s\\n]*id=\"front-cover\"");
    if (infos.contains(MovieScraperInfo::Poster) && rx.indexIn(html) != -1) {
        Poster p;
        p.thumbUrl = rx.cap(1);
        p.originalUrl = rx.cap(1);
        movie->images().addPoster(p);
    }

    rx.setPattern(R"(<a href="[^"]*"[\s\r\n]*Category="Item Page" Label="Series">[\s\r\n]*([^<]*)<span)");
    if (infos.contains(MovieScraperInfo::Set) && rx.indexIn(html) != -1) {
        doc.setHtml(rx.cap(1));
        QString setName = doc.toPlainText().trimmed();
        if (setName.endsWith("Series", Qt::CaseInsensitive)) {
            setName.chop(6);
        }
        setName = setName.trimmed();
        if (setName.startsWith("\"")) {
            setName.remove(0, 1);
        }
        if (setName.endsWith("\"")) {
            setName.chop(1);
        }
        MovieSet set;
        set.name = setName.trimmed();
        movie->setSet(set);
    }

    if (infos.contains(MovieScraperInfo::Backdrop)) {
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

void AdultDvdEmpire::loadSettings(ScraperSettings& settings)
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

} // namespace scraper
} // namespace mediaelch
