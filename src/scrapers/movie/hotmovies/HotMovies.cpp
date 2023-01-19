#include "HotMovies.h"

#include "data/movie/Movie.h"
#include "scrapers/movie/hotmovies/HotMoviesSearchJob.h"
#include "settings/Settings.h"

#include <QGridLayout>
#include <QRegularExpression>
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

MovieSearchJob* HotMovies::search(MovieSearchJob::Config config)
{
    return new HotMoviesSearchJob(m_api, std::move(config), this);
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

void HotMovies::loadData(QHash<MovieScraper*, mediaelch::scraper::MovieIdentifier> ids,
    Movie* movie,
    QSet<MovieScraperInfo> infos)
{
    if (ids.isEmpty()) {
        // TODO: Should not happen.
        return;
    }

    m_api.loadMovie(ids.constBegin().value().str(), [movie, infos, this](QString data, ScraperError error) {
        movie->clear(infos);

        if (!error.hasError()) {
            parseAndAssignInfos(data, movie, infos);

        } else {
            // TODO
            showNetworkError(error);
        }

        movie->controller()->scraperLoadDone(this, error);
    });
}

void HotMovies::parseAndAssignInfos(QString html, Movie* movie, QSet<MovieScraperInfo> infos)
{
    QRegularExpression rx;
    rx.setPatternOptions(QRegularExpression::InvertedGreedinessOption | QRegularExpression::DotMatchesEverythingOption);
    QRegularExpressionMatch match;

    rx.setPattern(R"(<h1 class="[^"]+"(?: itemprop="name")?>(.*)</h1>)");
    match = rx.match(html);
    if (infos.contains(MovieScraperInfo::Title) && match.hasMatch()) {
        movie->setName(decodeAndTrim(match.captured(1)));
    }

    // Rating currently not available; HotMovies has switched to likes
    // rx.setPattern("<meta itemprop=\"ratingValue\" content=\"(.*)\">");
    // if (infos.contains(MovieScraperInfo::Rating) && match.hasMatch()) {
    //     movie->setRating(match.captured(1).toDouble());
    // }

    // Only the main like count has text after the thumbs-up-count
    // In 2019, it contained a link (therefore `</a>`).
    // As of 2020-04-05 this is was the case anymore but as of 2022-06-05 it's there again.
    rx.setPattern(R"(<span class="thumbs-up-count">(\d+)</span>(</a>)?<br /><span class="thumbs-up-text")");
    match = rx.match(html);
    if (infos.contains(MovieScraperInfo::Rating) && match.hasMatch()) {
        Rating rating;
        rating.voteCount = match.captured(1).toInt();
        rating.source = "HotMovies";
        movie->ratings().setOrAddRating(rating);
    }

    rx.setPattern("<strong>Released:</strong> ?([0-9]{4})");
    match = rx.match(html);
    if (infos.contains(MovieScraperInfo::Released) && match.hasMatch()) {
        movie->setReleased(QDate::fromString(match.captured(1), "yyyy"));
    }

    rx.setPattern(R"(<span(?: itemprop="duration")? datetime="PT[^"]+">([^<]*)</span>)");
    match = rx.match(html);
    if (infos.contains(MovieScraperInfo::Runtime) && match.hasMatch()) {
        using namespace std::chrono;
        QStringList runtimeStr = match.captured(1).split(":");
        if (runtimeStr.count() == 3) {
            minutes runtime = hours(runtimeStr.at(0).toInt()) + minutes(runtimeStr.at(1).toInt());
            movie->setRuntime(runtime);

        } else if (runtimeStr.count() == 2) {
            minutes runtime = minutes(runtimeStr.at(0).toInt());
            movie->setRuntime(runtime);
        }
    }

    rx.setPattern(R"(<article>.*</h2>(.*)</article>)");
    match = rx.match(html);
    if (infos.contains(MovieScraperInfo::Overview) && match.hasMatch()) {
        movie->setOverview(decodeAndTrim(match.captured(1)));

        if (Settings::instance()->usePlotForOutline()) {
            movie->setOutline(movie->overview());
        }
    }

    rx.setPattern(R"rx(data-front="([^"]+)")rx");
    match = rx.match(html);
    if (infos.contains(MovieScraperInfo::Poster) && match.hasMatch()) {
        Poster p;
        p.thumbUrl = match.captured(1);
        p.originalUrl = match.captured(1);
        movie->images().addPoster(p);
    }

    rx.setPattern(R"rx(data-back="([^"]+)")rx");
    match = rx.match(html);
    if (infos.contains(MovieScraperInfo::Backdrop) && match.hasMatch()) {
        Poster p;
        p.thumbUrl = match.captured(1);
        p.originalUrl = match.captured(1);
        movie->images().addBackdrop(p);
    }

    rx.setPattern( R"re(<strong>Starring:</strong>(.*)</div>)re");
    match = rx.match(html);
    if (infos.contains(MovieScraperInfo::Actors) && match.hasMatch()) {
        // clear actors
        movie->setActors({});
        rx.setPattern( R"re(Label="Performer">(.*)</a>)re");
        QRegularExpressionMatchIterator matches = rx.globalMatch(match.captured(1));
        while (matches.hasNext()) {
            match = matches.next();
            Actor a;
            a.name = decodeAndTrim(decodeAndTrim(match.captured(1)));
            const auto pictureUrl = match.captured(1);
            //if (!pictureUrl.endsWith("missing_f.gif") && !pictureUrl.endsWith("missing_m.gif")) {
            //    a.thumb = pictureUrl;
            //}
            movie->addActor(a);
        }
    }

    if (infos.contains(MovieScraperInfo::Genres)) {
        rx.setPattern("title=\"Plot Oriented -> ([^\"]+)\"");
        QRegularExpressionMatchIterator matches = rx.globalMatch(html);
        while (matches.hasNext()) {
            movie->addGenre(decodeAndTrim(matches.next().captured(1)));
        }
    }

    rx.setPattern(R"re(<strong>Studio:</strong> <a(?: itemprop="url")? href="[^"]*"[\s\t\n]*title="([^"]*)")re");
    match = rx.match(html);
    if (infos.contains(MovieScraperInfo::Studios) && match.hasMatch()) {
        movie->addStudio(decodeAndTrim(match.captured(1)));
    }

    rx.setPattern(R"re("director":\[\{"@type":"Person","name":"([^"]+)")re");
    match = rx.match(html);
    if (infos.contains(MovieScraperInfo::Director) && match.hasMatch()) {
        movie->setDirector(decodeAndTrim(match.captured(1)));
    }

    rx.setPattern(R"re(<strong>Series:</strong>\s+<a href="/series/[^"]+">(.*)</a>)re");
    match = rx.match(html);
    if (infos.contains(MovieScraperInfo::Set) && match.hasMatch()) {
        MovieSet set;
        set.name = decodeAndTrim(match.captured(1));
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

QString HotMovies::decodeAndTrim(const QString& htmlEncodedString)
{
    return QTextDocumentFragment::fromHtml(htmlEncodedString).toPlainText().trimmed();
}

} // namespace scraper
} // namespace mediaelch
