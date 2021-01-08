#include "HotMovies.h"

#include "data/Storage.h"
#include "globals/Helper.h"
#include "network/NetworkRequest.h"
#include "ui/main/MainWindow.h"

#include <QDebug>
#include <QGridLayout>
#include <QRegExp>
#include <QTextDocument>
#include <QTextDocumentFragment>

namespace mediaelch {
namespace scraper {

HotMovies::HotMovies(QObject* parent) : MovieScraper(parent)
{
    m_meta.identifier = ID;
    m_meta.name = "HotMovies";
    m_meta.description = "HotMovies is a video database for adult content.";
    m_meta.website = "https://www.hotmovies.com";
    m_meta.termsOfService = "https://www.hotmovies.com";
    m_meta.privacyPolicy = "https://www.hotmovies.com";
    m_meta.help = "https://www.hotmovies.com";
    m_meta.supportedDetails = {MovieScraperInfo::Title,
        MovieScraperInfo::Rating,
        MovieScraperInfo::Released,
        MovieScraperInfo::Runtime,
        MovieScraperInfo::Overview,
        MovieScraperInfo::Poster,
        MovieScraperInfo::Backdrop,
        MovieScraperInfo::Actors,
        MovieScraperInfo::Genres,
        MovieScraperInfo::Studios,
        MovieScraperInfo::Director,
        MovieScraperInfo::Set};
    m_meta.supportedLanguages = {"en"};
    m_meta.defaultLocale = "en";
    m_meta.isAdult = true;
}

const MovieScraper::ScraperMeta& HotMovies::meta() const
{
    return m_meta;
}

void HotMovies::initialize()
{
    // no-op
    // HotMovies requires no initialization.
}

bool HotMovies::isInitialized() const
{
    // HotMovies requires no initialization.
    return true;
}

QSet<MovieScraperInfo> HotMovies::scraperNativelySupports()
{
    return m_meta.supportedDetails;
}

void HotMovies::changeLanguage(mediaelch::Locale /*locale*/)
{
    // no-op: Only one language is supported and it is hard-coded.
}

mediaelch::network::NetworkManager* HotMovies::network()
{
    return &m_network;
}

void HotMovies::search(QString searchStr)
{
    m_api.searchForMovie(searchStr, [this](QString data, ScraperError error) {
        if (error.hasError()) {
            qWarning() << "[HotMovies] Search Error" << error.message << "|" << error.technical;
            emit searchDone({}, error);

        } else {
            emit searchDone(parseSearch(data), {});
        }
    });
}

QVector<ScraperSearchResult> HotMovies::parseSearch(QString html)
{
    QVector<ScraperSearchResult> results;
    int offset = 0;

    QRegExp rx(R"lit(<div class="cell td_title">.*<h3 class="title">.*<a href="([^"]*)" title="[^"]*">(.*)</a>)lit");
    rx.setMinimal(true);
    while ((offset = rx.indexIn(html, offset)) != -1) {
        ScraperSearchResult result;
        result.id = rx.cap(1);
        result.name = QTextDocumentFragment::fromHtml(rx.cap(2)).toPlainText().trimmed();
        results << result;
        offset += rx.matchedLength();
    }

    return results;
}

void HotMovies::loadData(QHash<MovieScraper*, QString> ids, Movie* movie, QSet<MovieScraperInfo> infos)
{
    m_api.loadMovie(ids.values().first(), [movie, infos, this](QString data, ScraperError error) {
        movie->clear(infos);

        if (!error.hasError()) {
            parseAndAssignInfos(data, movie, infos);

        } else {
            // TODO
            showNetworkError(error);
        }

        movie->controller()->scraperLoadDone(this);
    });
}

void HotMovies::parseAndAssignInfos(QString html, Movie* movie, QSet<MovieScraperInfo> infos)
{
    QRegExp rx;
    rx.setMinimal(true);

    rx.setPattern(R"(<h1 class="title"(?: itemprop="name")?>(.*)</h1>)");
    if (infos.contains(MovieScraperInfo::Title) && rx.indexIn(html) != -1) {
        movie->setName(rx.cap(1));
    }

    // Rating currently not available; HotMovies has switched to likes
    // rx.setPattern("<meta itemprop=\"ratingValue\" content=\"(.*)\">");
    // if (infos.contains(MovieScraperInfo::Rating) && rx.indexIn(html) != -1) {
    //     movie->setRating(rx.cap(1).toDouble());
    // }

    // Only the main like count has text after the thumbs-up-count
    // In 2019, it contained a link (therefore `</a>`).
    // As of 2020-04-05 this is not the case anymore.
    rx.setPattern(R"(<span class="thumbs-up-count">(\d+)</span>(</a>)?<br /><span class="thumbs-up-text">)");
    if (infos.contains(MovieScraperInfo::Rating) && rx.indexIn(html) != -1) {
        Rating rating;
        rating.voteCount = rx.cap(1).toInt();
        rating.source = "HotMovies";
        movie->ratings().push_back(rating);
    }

    rx.setPattern("<strong>Released:</strong> ?([0-9]{4})");
    if (infos.contains(MovieScraperInfo::Released) && rx.indexIn(html) != -1) {
        movie->setReleased(QDate::fromString(rx.cap(1), "yyyy"));
    }

    rx.setPattern(R"(<span(?: itemprop="duration")? datetime="PT[^"]+">([^<]*)</span>)");
    if (infos.contains(MovieScraperInfo::Runtime) && rx.indexIn(html) != -1) {
        using namespace std::chrono;
        QStringList runtimeStr = rx.cap(1).split(":");
        if (runtimeStr.count() == 3) {
            minutes runtime = hours(runtimeStr.at(0).toInt()) + minutes(runtimeStr.at(1).toInt());
            movie->setRuntime(runtime);

        } else if (runtimeStr.count() == 2) {
            minutes runtime = minutes(runtimeStr.at(0).toInt());
            movie->setRuntime(runtime);
        }
    }

    rx.setPattern(R"(<span class="video_description"(?: itemprop="description")?>(.*)</span>)");
    if (infos.contains(MovieScraperInfo::Overview) && rx.indexIn(html) != -1) {
        QTextDocument doc;
        doc.setHtml(rx.cap(1));
        movie->setOverview(doc.toPlainText().trimmed());

        if (Settings::instance()->usePlotForOutline()) {
            movie->setOutline(movie->overview());
        }
    }

    rx.setPattern(R"rx(data-front="([^"]+)")rx");
    if (infos.contains(MovieScraperInfo::Poster) && rx.indexIn(html) != -1) {
        Poster p;
        p.thumbUrl = rx.cap(1);
        p.originalUrl = rx.cap(1);
        movie->images().addPoster(p);
    }

    rx.setPattern(R"rx(data-back="([^"]+)")rx");
    if (infos.contains(MovieScraperInfo::Backdrop) && rx.indexIn(html) != -1) {
        Poster p;
        p.thumbUrl = rx.cap(1);
        p.originalUrl = rx.cap(1);
        movie->images().addBackdrop(p);
    }

    if (infos.contains(MovieScraperInfo::Actors)) {
        // clear actors
        movie->setActors({});
        //        rx.setPattern("key=\"([^\"]*)\"/> <img
        //        src=\"https://img[0-9]+.vod.com/image[0-9]?/vodimages/images/spacer.gif\" class=\"lg_star_image\"
        //        itemprop=\"image\" alt\"\" /> </span><a href=\"[^\"]*\".*[\\s\\n]*title=\"[^\"]*\" rel=\"tag\"
        //        itemprop=\"url\"><span itemprop=\"name\">(.*)<\/span></a>");
        //                      "title=\"[^\"]*\" rel=\"tag\" itemprop=\"actor\" itemscope
        //                      itemtype=\"https://schema.org/Person\"><span itemprop=\"name\">(.*)</span></a>");

        //        rx.setPattern("<div class=\"star_wrapper\" key=\"(.*)\"><a href=\".*\" .* title=\".*\" rel=\"tag\"
        //        itemprop=\"url\"><span itemprop=\"name\">(.*)</span></a></div>");
        rx.setPattern(
            R"re(<div class="star_wrapper" key="([^"]*)"><img [^>]*/><span(?: itemprop="name")?>([^<]*)</span>)re");
        rx.setMinimal(true);
        int offset = 0;
        while ((offset = rx.indexIn(html, offset)) != -1) {
            offset += rx.matchedLength();
            Actor a;
            a.name = rx.cap(2);
            const auto pictureUrl = rx.cap(1);
            if (!pictureUrl.endsWith("missing_f.gif") && !pictureUrl.endsWith("missing_m.gif")) {
                a.thumb = pictureUrl;
            }
            movie->addActor(a);
        }
    }

    if (infos.contains(MovieScraperInfo::Genres)) {
        rx.setPattern("title=\"Plot Oriented -> ([^\"]+)\"");
        int offset = 0;
        while ((offset = rx.indexIn(html, offset)) != -1) {
            offset += rx.matchedLength();
            movie->addGenre(rx.cap(1));
        }
    }

    rx.setPattern(R"re(<strong>Studio:</strong> <a(?: itemprop="url")? href="[^"]*"[\s\t\n]*title="([^"]*)")re");
    if (infos.contains(MovieScraperInfo::Studios) && rx.indexIn(html) != -1) {
        movie->addStudio(rx.cap(1));
    }

    rx.setPattern(R"re("director":\[\{"@type":"Person","name":"([^"]+)")re");
    if (infos.contains(MovieScraperInfo::Director) && rx.indexIn(html) != -1) {
        movie->setDirector(rx.cap(1));
    }

    // Title may contain `"` which results in invalid HTML.
    rx.setPattern(R"(<a href="https://www.hotmovies.com/series/[^"]*" title=".*" rel="tag">(.*)</a>)");
    if (infos.contains(MovieScraperInfo::Set) && rx.indexIn(html) != -1) {
        MovieSet set;
        set.name = rx.cap(1);
        movie->setSet(set);
    }
}

bool HotMovies::hasSettings() const
{
    return false;
}

void HotMovies::loadSettings(ScraperSettings& settings)
{
    Q_UNUSED(settings);
}

void HotMovies::saveSettings(ScraperSettings& settings)
{
    Q_UNUSED(settings);
}

QWidget* HotMovies::settingsWidget()
{
    return nullptr;
}

} // namespace scraper
} // namespace mediaelch
