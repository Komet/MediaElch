#include "scrapers/tv_show/fernsehserien_de/FernsehserienDe.h"

#include "data/tv_show/TvShow.h"
#include "data/tv_show/TvShowEpisode.h"
#include "log/Log.h"
#include "network/NetworkRequest.h"
#include "scrapers/ScraperUtils.h"
#include "scrapers/tv_show/ShowMerger.h"

#include <QRegularExpression>
#include <QTimer>

namespace mediaelch {
namespace scraper {

namespace {

// Used for GET requests. Fernsehserien.de is a bit slow on the first request.
constexpr int defaultNetworkTimeoutSeconds = 30;

/// \brief Parses episodes of a TV show overview page via iterator like interface.
struct EpisodeListParser
{
public:
    static EpisodeListParser forHtml(const QString& html)
    {
        static QRegularExpression resultEntry(
            R"re(<a role="row" data-event-category="liste-episoden" href="/([^"]+?)" title="([^"]+?)")re",
            QRegularExpression::DotMatchesEverythingOption);
        MediaElch_Debug_Ensures(resultEntry.isValid());

        EpisodeListParser parser;
        parser.m_matches = resultEntry.globalMatch(html);
        parser.next();
        return parser;
    }

public:
    struct Result
    {
        SeasonNumber seasonNumber;
        EpisodeNumber episodeNumber;
        QString id;

        bool isValid() const { return !id.isEmpty(); }
    };

public:
    bool hasNext() const { return m_next.isValid(); }

    Result next()
    {
        static QRegularExpression episodeSeason(R"(^(\d+)[.](\d+))");
        MediaElch_Debug_Ensures(episodeSeason.isValid());

        Result current = m_next;
        m_next = {};

        if (m_matches.hasNext()) {
            QRegularExpressionMatch match = m_matches.next();

            QString id = match.captured(1);
            QString title = match.captured(2);
            match = episodeSeason.match(title);

            if (match.hasMatch()) {
                bool seasonOk = false;
                bool episodeOk = false;
                auto season = SeasonNumber(match.captured(1).toInt(&seasonOk, 10));
                auto episode = EpisodeNumber(match.captured(2).toInt(&episodeOk, 10));

                if (seasonOk && episodeOk) {
                    m_next = Result{season, episode, id};
                }
            }
        }

        return current;
    }

private:
    explicit EpisodeListParser() = default;

    Result m_next;
    QRegularExpressionMatchIterator m_matches;
};

void normalizeActorRole(QString& role)
{
    // \u200B: NO WIDTH SPACE
    role.remove("Originalsprecher");
    role.replace(QString::fromUtf8(" /\u200B"), ",");
}

bool isFernsehserienDeActorRole(const QString& role)
{
    return role != "Drehbuch" && role != "Regie";
}

} // namespace

QUrl FernsehserienDeApi::searchUrl(const QString& query)
{
    return QUrl{QStringLiteral("https://www.fernsehserien.de/suche/%1") //
            .arg(QString(QUrl::toPercentEncoding(query)))};
}

QUrl FernsehserienDeApi::tvShowUrl(const ShowIdentifier& id)
{
    return QUrl{QStringLiteral("https://www.fernsehserien.de/%1").arg(id.str())};
}

QUrl FernsehserienDeApi::episodeUrl(const QString& id)
{
    return QUrl{QStringLiteral("https://www.fernsehserien.de/%1").arg(id)};
}

QUrl FernsehserienDeApi::seasonsOverviewUrl(const ShowIdentifier& id)
{
    // Season number is not used, because fernsehserien.de uses an internal
    // ID for it, which we don't know at this point.
    return QUrl{QStringLiteral("https://www.fernsehserien.de/%1/episodenguide").arg(id.str())};
}

QString FernsehserienDe::ID = "fernsehserien.de";

FernsehserienDe::FernsehserienDe(FernsehserienDeConfiguration& settings, QObject* parent) :
    TvScraper(parent), m_settings{settings}
{
    m_meta.identifier = FernsehserienDe::ID;
    m_meta.name = "Fernsehserien TV";
    m_meta.description = tr("fernsehserien.de is a German TV show catalog and news portal.");
    m_meta.website = "https://www.fernsehserien.de/";
    m_meta.termsOfService = "https://www.fernsehserien.de/impressum";
    m_meta.privacyPolicy = "https://www.fernsehserien.de/datenschutz";
    m_meta.help = "https://www.fernsehserien.de/";
    // or e.g. mediaelch::allShowScraperInfos();
    m_meta.supportedShowDetails = {
        ShowScraperInfo::Title,
        ShowScraperInfo::Genres,
        ShowScraperInfo::FirstAired,
        // also country
        ShowScraperInfo::Overview,
        ShowScraperInfo::Actors,
        ShowScraperInfo::Banner,
        ShowScraperInfo::Poster,
    };
    // or e.g. mediaelch::allEpisodeScraperInfos();
    m_meta.supportedEpisodeDetails = {
        EpisodeScraperInfo::Title,
        EpisodeScraperInfo::Actors,
        // also episode runtime
        // also original title
        // also producers
        EpisodeScraperInfo::FirstAired,
        EpisodeScraperInfo::Overview,
        EpisodeScraperInfo::Director,
        EpisodeScraperInfo::Thumbnail,
    };

    m_meta.supportedSeasonOrders = {SeasonOrder::Aired};
    m_meta.supportedLanguages = {Locale("de-DE")};
    m_meta.defaultLocale = Locale("de-DE");
}

const FernsehserienDe::ScraperMeta& FernsehserienDe::meta() const
{
    return m_meta;
}

void FernsehserienDe::initialize()
{
    QTimer::singleShot(0, this, [this]() { emit initialized(true, this); });
}

bool FernsehserienDe::isInitialized() const
{
    return true;
}

ShowSearchJob* FernsehserienDe::search(ShowSearchJob::Config config)
{
    return new FernsehserienDeShowSearchJob(m_api, config, this);
}

ShowScrapeJob* FernsehserienDe::loadShow(ShowScrapeJob::Config config)
{
    return new FernsehserienDeShowScrapeJob(m_api, config, this);
}

SeasonScrapeJob* FernsehserienDe::loadSeasons(SeasonScrapeJob::Config config)
{
    return new FernsehserienDeSeasonScrapeJob(m_api, config, this);
}

EpisodeScrapeJob* FernsehserienDe::loadEpisode(EpisodeScrapeJob::Config config)
{
    return new FernsehserienDeEpisodeScrapeJob(m_api, config, this);
}

FernsehserienDeShowSearchJob::FernsehserienDeShowSearchJob(FernsehserienDeApi& api,
    ShowSearchJob::Config _config,
    QObject* parent) :
    ShowSearchJob(_config, parent), m_api{api}
{
}

void FernsehserienDeShowSearchJob::doStart()
{
    // TODO: Fast search (uses JSON)

    QUrl url = m_api.searchUrl(config().query);
    QNetworkRequest request = mediaelch::network::requestWithDefaults(url);
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::ManualRedirectPolicy);

    if (m_api.network().cache().hasValidElement(request)) {
        // Note: Currently, only /suche/ URLs are cached.
        // TODO: Cache direct TV show page as well; determine which page we have somehow.
        QString html = m_api.network().cache().getElement(request);
        m_results = parseSearch(html);
        emitFinished();

    } else {
        // Use an increased timeout: Some search pages take >10s for the first time they are loaded.
        QNetworkReply* reply =
            m_api.network().getWithTimeout(request, std::chrono::seconds{defaultNetworkTimeoutSeconds});

        // TODO: Use this when we require Qt 5.9 or later
        // connect(reply, &QNetworkReply::redirected, this, [reply](const QUrl& url) {
        //     if (url.host() == "fernsehserien.de" || url.host() == "www.fernsehserien.de") {
        //         reply->redirectAllowed();
        //     } else {
        //         reply->close();
        //     }
        // });

        connect(reply, &QNetworkReply::finished, this, &FernsehserienDeShowSearchJob::onSearchPageLoaded);
    }
}


void FernsehserienDeShowSearchJob::onSearchPageLoaded()
{
    auto* reply = dynamic_cast<QNetworkReply*>(QObject::sender());
    MediaElch_Debug_Ensures(reply != nullptr);
    auto dls = makeDeleteLaterScope(reply);

    const int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if (statusCode == 301 || statusCode == 302) {
        // Redirects to unsafe HTTP:
        //   - https://www.fernsehserien.de/suche/Scrubs
        //   - http://www.fernsehserien.de/scrubs
        //   - https://www.fernsehserien.de:443/scrubs

        ++m_redirectionCount;
        if (m_redirectionCount > 5) {
            ScraperError error;
            error.error = ScraperError::Type::NetworkError;
            error.message = translateNetworkError(QNetworkReply::TooManyRedirectsError);
            error.technical = "Too many HTTP redirections.";
            setScraperError(error);
            emitFinished();
            return;
        }

        // Here, we skip the redirection to the unsafe HTTP version.
        QUrl redirectedUrl = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();
        redirectedUrl.setScheme("https");
        redirectedUrl.setHost("www.fernsehserien.de");
        redirectedUrl.setPort(443);

        QNetworkRequest request = mediaelch::network::requestWithDefaults(redirectedUrl);
        request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::ManualRedirectPolicy);

        QNetworkReply* redirectedReply =
            m_api.network().getWithTimeout(request, std::chrono::seconds{defaultNetworkTimeoutSeconds});
        connect(redirectedReply, &QNetworkReply::finished, this, &FernsehserienDeShowSearchJob::onSearchPageLoaded);

    } else {
        QString html;
        if (reply->error() == QNetworkReply::NoError) {
            html = QString::fromUtf8(reply->readAll());
        }

        ScraperError error = makeScraperError(html, *reply, {});
        setScraperError(error);

        if (!error.hasError()) {
            // It could be that fernsehserien.de has redirected us to the TV show page,
            // instead of the search result page.
            if (reply->url().path().contains("/suche/")) {
                m_api.network().cache().addElement(reply->request(), html);
                m_results = parseSearch(html);

            } else {
                // TODO: Cache here as well
                m_results.push_back(parseResultFromEpisodePage(reply->url(), html));
            }
        }

        emitFinished();
    }
}

QVector<ShowSearchJob::Result> FernsehserienDeShowSearchJob::parseSearch(const QString& html)
{
    // The HTML of fernsehserien.de is not valid XML, so we can't rely on QDomDocument
    // or another form of XML parsing.

    static QRegularExpression resultList(
        R"(<ul class="suchergebnisse"[^>]*>.*?</ul>)", QRegularExpression::DotMatchesEverythingOption);
    static QRegularExpression resultEntry(R"(<li[^>]+>.*?</li>)", QRegularExpression::DotMatchesEverythingOption);
    static QRegularExpression resultTitle(R"(<dt>(.*?)</dt>)", QRegularExpression::DotMatchesEverythingOption);
    static QRegularExpression resultYear(
        R"(<dd>.*?(\d{4})[-– ].*?</dd>)", QRegularExpression::DotMatchesEverythingOption);
    static QRegularExpression resultId(R"re(href="/(.*?)")re", QRegularExpression::DotMatchesEverythingOption);

    MediaElch_Debug_Ensures(resultList.isValid());
    MediaElch_Debug_Ensures(resultEntry.isValid());
    MediaElch_Debug_Ensures(resultTitle.isValid());
    MediaElch_Debug_Ensures(resultYear.isValid());
    MediaElch_Debug_Ensures(resultId.isValid());

    QRegularExpressionMatch listMatch = resultList.match(html);
    if (!listMatch.hasMatch()) {
        return {};
    }

    QVector<ShowSearchJob::Result> results;
    QRegularExpressionMatchIterator matches = resultEntry.globalMatch(listMatch.captured(0));

    while (matches.hasNext()) {
        const QString entry = matches.next().captured(0);

        ShowSearchJob::Result result;
        result.title = normalizeFromHtml(resultTitle.match(entry).captured(1));
        result.released = QDate::fromString(resultYear.match(entry).captured(1), "yyyy");
        result.identifier = ShowIdentifier(resultId.match(entry).captured(1));

        results << result;
    }

    return results;
}

ShowSearchJob::Result FernsehserienDeShowSearchJob::parseResultFromEpisodePage(const QUrl& url, const QString& html)
{
    static QRegularExpression resultTitle(
        R"(<div class="seriestitle">(.*?)</div>)", QRegularExpression::DotMatchesEverythingOption);
    static QRegularExpression resultYear(
        R"(<dd>.*?(\d{4})[-– ].*?</dd>)", QRegularExpression::DotMatchesEverythingOption);

    MediaElch_Debug_Ensures(resultTitle.isValid());
    MediaElch_Debug_Ensures(resultYear.isValid());
    MediaElch_Debug_Ensures(url.isValid());

    QString identifierUrl = url.path();
    MediaElch_Debug_Assert(identifierUrl.startsWith('/'));

    // Add a dash between title and addendum. Typical German thing.  For example, "Scrubs"
    // is "Scrubs - Die Anfänger". "Die Anfänger" is in the second row on fernsehserien.de.
    QString title = resultTitle.match(html).captured(1);
    title.replace("<span>", "<span> - ");

    ShowSearchJob::Result result;
    result.title = normalizeFromHtml(title);
    result.released = QDate::fromString(resultYear.match(html).captured(1), "yyyy");
    result.identifier = ShowIdentifier(identifierUrl.remove(0, 1));

    return result;
}

FernsehserienDeShowScrapeJob::FernsehserienDeShowScrapeJob(FernsehserienDeApi& api,
    ShowScrapeJob::Config _config,
    QObject* parent) :
    ShowScrapeJob(_config, parent), m_api{api}
{
}

void FernsehserienDeShowScrapeJob::doStart()
{
    QUrl url = m_api.tvShowUrl(config().identifier);
    QNetworkRequest request = mediaelch::network::requestWithDefaults(url);

    if (m_api.network().cache().hasValidElement(request)) {
        QString html = m_api.network().cache().getElement(request);
        parseTvShow(html);
        emitFinished();

    } else {
        QNetworkReply* reply =
            m_api.network().getWithTimeout(request, std::chrono::seconds{defaultNetworkTimeoutSeconds});

        connect(reply, &QNetworkReply::finished, this, [reply, request, this]() {
            auto dls = makeDeleteLaterScope(reply);
            QString html;
            if (reply->error() == QNetworkReply::NoError) {
                html = QString::fromUtf8(reply->readAll());
            }

            ScraperError error = makeScraperError(html, *reply, {});
            setScraperError(error);

            if (!error.hasError()) {
                m_api.network().cache().addElement(request, html);
                parseTvShow(html);
            }

            emitFinished();
        });
    }
}

void FernsehserienDeShowScrapeJob::parseTvShow(const QString& html)
{
    static QRegularExpression titleRegEx(
        R"(<div class="seriestitle">(.*?)</div>)", QRegularExpression::DotMatchesEverythingOption);
    static QRegularExpression originalTitleRegEx(R"re(<span lang="[^"]+?" itemprop="alternateName">(.*?)</span>)re",
        QRegularExpression::DotMatchesEverythingOption);
    static QRegularExpression overviewRegEx(
        R"("serie-beschreibung"[^>]*>(.*?)</div><)", QRegularExpression::DotMatchesEverythingOption);
    // Note: There are possibly multiple "first aired"-dates ("Premiere"). Take the first, which
    //       is most likely the German one.
    static QRegularExpression firstAiredRegEx(R"re(<ea-angabe-datum><time datetime="(\d{4}-\d{2}-\d{2})">)re");
    static QRegularExpression genresRegEx(R"re(<meta itemprop="genre" content="([^"]+?)">)re");

    static QRegularExpression actorsRegEx(
        R"re(data-event-category="liste-cast-crew".*?</a>)re", QRegularExpression::DotMatchesEverythingOption);
    static QRegularExpression actorNameRegEx(
        R"re(<dt itemprop="name">(.*?)</dt>)re", QRegularExpression::DotMatchesEverythingOption);
    static QRegularExpression actorRoleRegEx(R"re(<dd>(.*?)</dd>)re", QRegularExpression::DotMatchesEverythingOption);
    static QRegularExpression actorImageRegEx(
        R"re(data-src="(https://bilder.fernsehserien.de/[^"]+?)")re", QRegularExpression::DotMatchesEverythingOption);

    // Poster is more of a thumbnail.
    static QRegularExpression posterRegEx(
        R"re(<meta itemprop="image" content="(https://bilder.fernsehserien.de/gfx/logo/[^"]+?[.](?:png|jpg|jpeg))")re");
    static QRegularExpression bannerRegEx(
        R"re(<meta itemprop="image" content="(https://bilder.fernsehserien.de/sendung/hr/[^"]+?[.](?:png|jpg|jpeg))")re");

    MediaElch_Debug_Ensures(titleRegEx.isValid());
    MediaElch_Debug_Ensures(originalTitleRegEx.isValid());
    MediaElch_Debug_Ensures(overviewRegEx.isValid());
    MediaElch_Debug_Ensures(firstAiredRegEx.isValid());
    MediaElch_Debug_Ensures(genresRegEx.isValid());
    MediaElch_Debug_Ensures(actorsRegEx.isValid());
    MediaElch_Debug_Ensures(actorNameRegEx.isValid());
    MediaElch_Debug_Ensures(actorRoleRegEx.isValid());
    MediaElch_Debug_Ensures(actorImageRegEx.isValid());
    MediaElch_Debug_Ensures(posterRegEx.isValid());
    MediaElch_Debug_Ensures(bannerRegEx.isValid());

    // Add a dash between title and addendum. Typical German thing.  For example, "Scrubs"
    // is "Scrubs - Die Anfänger". "Die Anfänger" is in the second row on fernsehserien.de.
    QString seriesTitle = titleRegEx.match(html).captured(1);
    seriesTitle.replace("<span>", "<span> - ");

    tvShow().setTitle(normalizeFromHtml(seriesTitle));
    tvShow().setOriginalTitle(normalizeFromHtml(originalTitleRegEx.match(html).captured(1)));
    tvShow().setOverview(normalizeFromHtml(overviewRegEx.match(html).captured(1)));
    tvShow().setFirstAired(QDate::fromString(firstAiredRegEx.match(html).captured(1), "yyyy-MM-dd"));

    QRegularExpressionMatchIterator matches = genresRegEx.globalMatch(html);
    while (matches.hasNext()) {
        const QString genre = normalizeFromHtml(matches.next().captured(1));
        tvShow().addGenre(genre);
    }

    matches = actorsRegEx.globalMatch(html);
    while (matches.hasNext()) {
        const QString actorHtml = matches.next().captured(0);

        QString roles = actorRoleRegEx.match(actorHtml).captured(1);
        normalizeActorRole(roles);

        Actor actor;
        actor.name = normalizeFromHtml(actorNameRegEx.match(actorHtml).captured(1));
        actor.role = normalizeFromHtml(roles);
        QString thumb = actorImageRegEx.match(actorHtml).captured(1);
        if (!thumb.endsWith(".svg")) { // e.g. Person.svg, i.e. placeholder
            actor.thumb = normalizeFromHtml(thumb);
        }

        if (isFernsehserienDeActorRole(actor.role)) {
            tvShow().addActor(actor);
        }
    }

    QRegularExpressionMatch imageMatch = bannerRegEx.match(html);
    if (imageMatch.hasMatch()) {
        Poster banner;
        banner.originalUrl = imageMatch.captured(1);
        banner.language = "de-DE";
        tvShow().addBanner(banner);
    }

    imageMatch = posterRegEx.match(html);
    if (imageMatch.hasMatch()) {
        Poster poster;
        poster.originalUrl = imageMatch.captured(1);
        poster.language = "de-DE";
        tvShow().addPoster(poster);
    }
}

FernsehserienDeSeasonScrapeJob::FernsehserienDeSeasonScrapeJob(FernsehserienDeApi& api,
    SeasonScrapeJob::Config _config,
    QObject* parent) :
    SeasonScrapeJob(_config, parent), m_api{api}
{
}

void FernsehserienDeSeasonScrapeJob::doStart()
{
    QUrl url = m_api.seasonsOverviewUrl(config().showIdentifier);
    QNetworkRequest request = mediaelch::network::requestWithDefaults(url);
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::UserVerifiedRedirectPolicy);

    if (m_api.network().cache().hasValidElement(request)) {
        QString html = m_api.network().cache().getElement(request);
        parseSeasonPageAndStartLoading(html);

    } else {
        QNetworkReply* reply =
            m_api.network().getWithTimeout(request, std::chrono::seconds{defaultNetworkTimeoutSeconds});
        // Due to redirection, we may lose the original request otherwise.
        reply->setProperty("original_request", QVariant::fromValue(request));
        connect(reply, &QNetworkReply::finished, this, &FernsehserienDeSeasonScrapeJob::onSeasonPageLoaded);
    }
}

void FernsehserienDeSeasonScrapeJob::onSeasonPageLoaded()
{
    auto* reply = dynamic_cast<QNetworkReply*>(QObject::sender());
    MediaElch_Debug_Ensures(reply != nullptr);
    auto dls = makeDeleteLaterScope(reply);

    const int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if (statusCode == 301 || statusCode == 302) {
        // Redirects to unsafe HTTP:
        //   - https://www.fernsehserien.de/die-simpsons/episodenguide/
        //   - http://www.fernsehserien.de/die-simpsons/episodenguide
        //   - https://www.fernsehserien.de:443/die-simpsons/episodenguide

        ++m_redirectionCount;
        if (m_redirectionCount > 5) {
            ScraperError error;
            error.error = ScraperError::Type::NetworkError;
            error.message = translateNetworkError(QNetworkReply::TooManyRedirectsError);
            error.technical = "Too many HTTP redirections.";
            setScraperError(error);
            emitFinished();
            return;
        }

        // Here, we skip the redirection to the unsafe HTTP version.
        QUrl redirectedUrl = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();
        redirectedUrl.setScheme("https");
        redirectedUrl.setHost("www.fernsehserien.de");
        redirectedUrl.setPort(443);

        QNetworkRequest request = mediaelch::network::requestWithDefaults(redirectedUrl);
        request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::ManualRedirectPolicy);

        QNetworkReply* redirectedReply =
            m_api.network().getWithTimeout(request, std::chrono::seconds{defaultNetworkTimeoutSeconds});
        // Due to redirection, we may lose the original request otherwise.
        redirectedReply->setProperty("original_request", QVariant::fromValue(reply->request()));
        connect(redirectedReply, &QNetworkReply::finished, this, &FernsehserienDeSeasonScrapeJob::onSeasonPageLoaded);

    } else {
        QString html;
        if (reply->error() == QNetworkReply::NoError) {
            html = QString::fromUtf8(reply->readAll());
        }

        ScraperError error = makeScraperError(html, *reply, {});
        setScraperError(error);

        if (error.hasError()) {
            emitFinished();

        } else {
            MediaElch_Debug_Assert(reply->property("original_request").canConvert<QNetworkRequest>());
            auto originalRequest = reply->property("original_request").value<QNetworkRequest>();
            m_api.network().cache().addElement(originalRequest, html);
            parseSeasonPageAndStartLoading(html);
        }
    }
}

void FernsehserienDeSeasonScrapeJob::parseSeasonPageAndStartLoading(const QString& html)
{
    QVector<FernsehserienDeSeasonScrapeJob::EpisodeEntry> allEpisodes;

    EpisodeListParser parser = EpisodeListParser::forHtml(html);
    while (parser.hasNext()) {
        EpisodeListParser::Result next = parser.next();
        allEpisodes.push_back(EpisodeEntry{next.seasonNumber, next.episodeNumber, next.id});
    }

    if (config().shouldLoadAllSeasons()) {
        m_episodeIds = allEpisodes;
    } else {
        // Use only those seasons that are actually requested
        std::copy_if(allEpisodes.begin(),
            allEpisodes.end(),
            std::back_inserter(m_episodeIds),
            [this](const EpisodeEntry& entry) { return config().seasons.contains(entry.seasonNumber); });
    }

    loadNextEpisode();
}

void FernsehserienDeSeasonScrapeJob::loadNextEpisode()
{
    if (m_episodeIds.isEmpty()) {
        emitFinished();
        return;
    }

    EpisodeEntry next = m_episodeIds.takeFirst();
    EpisodeIdentifier id(next.id);
    FernsehserienDeEpisodeScrapeJob::Config episodeConfig(id, config().locale, config().details);

    auto* nextJob = new FernsehserienDeEpisodeScrapeJob(m_api, episodeConfig, this);
    connect(nextJob, &FernsehserienDeEpisodeScrapeJob::loadFinished, this, [next, this](EpisodeScrapeJob* job) {
        auto dls = makeDeleteLaterScope(job);

        if (!job->hasError()) {
            auto* episode = new TvShowEpisode({}, this);
            copyDetailsToEpisode(*episode, job->episode(), job->config().details);
            episode->setSeason(next.seasonNumber);
            episode->setEpisode(next.episodeNumber);

            m_episodes[{episode->seasonNumber(), episode->episodeNumber()}] = episode;
        } else {
            // TODO: Some way to let the user know? Log for now:
            qCInfo(generic) << "[Fernsehserien] Could not scrape episode" << next.episodeNumber.toPaddedString()
                            << "of season" << next.seasonNumber.toPaddedString();
        }

        loadNextEpisode();
    });
    nextJob->start();
}

FernsehserienDeEpisodeScrapeJob::FernsehserienDeEpisodeScrapeJob(FernsehserienDeApi& api,
    EpisodeScrapeJob::Config _config,
    QObject* parent) :
    EpisodeScrapeJob(_config, parent), m_api{api}
{
}

void FernsehserienDeEpisodeScrapeJob::doStart()
{
    if (config().identifier.hasEpisodeIdentifier()) {
        loadEpisode(config().identifier.episodeIdentifier);
    } else {
        loadSeason();
    }
}

void FernsehserienDeEpisodeScrapeJob::loadSeason()
{
    episode().setSeason(config().identifier.seasonNumber);
    episode().setEpisode(config().identifier.episodeNumber);

    QString showId(config().identifier.showIdentifier);

    QUrl url = m_api.seasonsOverviewUrl(ShowIdentifier(showId));
    QNetworkRequest request = mediaelch::network::requestWithDefaults(url);
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::UserVerifiedRedirectPolicy);

    if (m_api.network().cache().hasValidElement(request)) {
        parseAndLoadEpisodeIdFromSeason(m_api.network().cache().getElement(request));

    } else {
        QNetworkReply* reply =
            m_api.network().getWithTimeout(request, std::chrono::seconds{defaultNetworkTimeoutSeconds});
        // Due to redirection, we may lose the original request otherwise.
        reply->setProperty("original_request", QVariant::fromValue(request));

        // TODO: Use this when we require Qt 5.9 or later
        // connect(reply, &QNetworkReply::redirected, this, [reply](const QUrl& redirectedUrl) {
        //     if (redirectedUrl.host() == "fernsehserien.de" || redirectedUrl.host() == "www.fernsehserien.de") {
        //         reply->redirectAllowed();
        //     } else {
        //         reply->close();
        //     }
        // });

        connect(reply, &QNetworkReply::finished, this, &FernsehserienDeEpisodeScrapeJob::onSeasonPageLoaded);
    }
}

void FernsehserienDeEpisodeScrapeJob::onSeasonPageLoaded()
{
    auto* reply = dynamic_cast<QNetworkReply*>(QObject::sender());
    MediaElch_Debug_Ensures(reply != nullptr);
    auto dls = makeDeleteLaterScope(reply);

    const int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if (statusCode == 301 || statusCode == 302) {
        // Redirects to unsafe HTTP:
        //   - https://www.fernsehserien.de/die-simpsons/episodenguide/
        //   - http://www.fernsehserien.de/die-simpsons/episodenguide
        //   - https://www.fernsehserien.de:443/die-simpsons/episodenguide

        ++m_redirectionCount;
        if (m_redirectionCount > 5) {
            ScraperError error;
            error.error = ScraperError::Type::NetworkError;
            error.message = translateNetworkError(QNetworkReply::TooManyRedirectsError);
            error.technical = "Too many HTTP redirections.";
            setScraperError(error);
            emitFinished();
            return;
        }

        // Here, we skip the redirection to the unsafe HTTP version.
        QUrl redirectedUrl = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();
        redirectedUrl.setScheme("https");
        redirectedUrl.setHost("www.fernsehserien.de");
        redirectedUrl.setPort(443);

        QNetworkRequest request = mediaelch::network::requestWithDefaults(redirectedUrl);
        request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::ManualRedirectPolicy);

        QNetworkReply* redirectedReply =
            m_api.network().getWithTimeout(request, std::chrono::seconds{defaultNetworkTimeoutSeconds});
        // Due to redirection, we may lose the original request otherwise.
        redirectedReply->setProperty("original_request", QVariant::fromValue(reply->request()));
        connect(redirectedReply, &QNetworkReply::finished, this, &FernsehserienDeEpisodeScrapeJob::onSeasonPageLoaded);

    } else {
        QString html;
        if (reply->error() == QNetworkReply::NoError) {
            html = QString::fromUtf8(reply->readAll());
        }

        ScraperError error = makeScraperError(html, *reply, {});
        setScraperError(error);

        if (error.hasError()) {
            emitFinished();

        } else {
            MediaElch_Debug_Assert(reply->property("original_request").canConvert<QNetworkRequest>());
            auto originalRequest = reply->property("original_request").value<QNetworkRequest>();
            m_api.network().cache().addElement(originalRequest, html);
            parseAndLoadEpisodeIdFromSeason(html);
        }
    }
}

void FernsehserienDeEpisodeScrapeJob::parseAndLoadEpisodeIdFromSeason(const QString& html)
{
    QString episodeId;
    EpisodeListParser parser = EpisodeListParser::forHtml(html);
    while (parser.hasNext()) {
        EpisodeListParser::Result next = parser.next();
        if (next.seasonNumber == config().identifier.seasonNumber
            && next.episodeNumber == config().identifier.episodeNumber) {
            episodeId = next.id;
            break;
        }
    }

    if (episodeId.isEmpty()) {
        ScraperError error;
        error.error = ScraperError::Type::ApiError;
        error.message = tr("Could not load ID for episode from season overview page. Can't scrape "
                           "requested TV show episode.");
        setScraperError(error);
        emitFinished();

    } else {
        loadEpisode(episodeId);
    }
}

void FernsehserienDeEpisodeScrapeJob::loadEpisode(const QString& episodeId)
{
    QUrl url = m_api.episodeUrl(episodeId);
    QNetworkRequest request = mediaelch::network::requestWithDefaults(url);

    if (m_api.network().cache().hasValidElement(request)) {
        QString html = m_api.network().cache().getElement(request);
        parseEpisode(html);
        emitFinished();

    } else {
        QNetworkReply* reply =
            m_api.network().getWithTimeout(request, std::chrono::seconds{defaultNetworkTimeoutSeconds});

        connect(reply, &QNetworkReply::finished, this, [reply, request, this]() {
            auto dls = makeDeleteLaterScope(reply);
            QString html;
            if (reply->error() == QNetworkReply::NoError) {
                html = QString::fromUtf8(reply->readAll());
            }

            ScraperError error = makeScraperError(html, *reply, {});
            if (!error.hasError()) {
                m_api.network().cache().addElement(request, html);
                parseEpisode(html);
            }

            setScraperError(error);
            emitFinished();
        });
    }
}

void FernsehserienDeEpisodeScrapeJob::parseEpisode(const QString& html)
{
    // TODO: With a proper HTML parser, we wouldn't need to do this!
    static QRegularExpression titleRegEx(
        R"(<span itemprop="name">(.*?)</span>)", QRegularExpression::DotMatchesEverythingOption);
    // Not supported by episodes, yet:
    // static QRegularExpression originalTitleRegEx(R"re(<span class="episode-output-originaltitel"
    //    lang="[^"]+?">[(]?(.*?)[)]?</span>)re");
    // Runtime not supported by episodes, yet.
    static QRegularExpression seasonEpisodeRegEx(
        R"re(itemprop="episodeNumber" content="\d+">Staffel (\d+), Folge (\d+) )re");
    static QRegularExpression overviewRegEx(
        R"(<div class="episode-output-inhalt-inner">(.*?)</div><ea)", QRegularExpression::DotMatchesEverythingOption);
    // Note: There are possibly multiple "first aired"-dates ("Premiere"). Take the first, which
    //       is most likely the German one.
    static QRegularExpression firstAiredRegEx(R"re(<ea-angabe-datum>.*?(\d{2}[.]\d{2}[.]\d{4})<)re");
    static QRegularExpression thumbDivRegEx(
        R"re(<div class="episode-output-inhalt"[^>]*>.*?</picture>)re", QRegularExpression::DotMatchesEverythingOption);
    static QRegularExpression thumbRegEx(
        R"re(<img src="(https://bilder.fernsehserien.de/[^"]+?)")re", QRegularExpression::DotMatchesEverythingOption);

    static QRegularExpression actorsRegEx(
        R"re(itemprop="actor">.*?</a>)re", QRegularExpression::DotMatchesEverythingOption);
    static QRegularExpression actorNameRegEx(
        R"re(<dt itemprop="name">(.*?)</dt>)re", QRegularExpression::DotMatchesEverythingOption);
    static QRegularExpression actorRoleRegEx(R"re(<dd>(.*?)</dd>)re", QRegularExpression::DotMatchesEverythingOption);
    static QRegularExpression actorImageRegEx(
        R"re(data-src="(https://bilder.fernsehserien.de/[^"]+?)")re", QRegularExpression::DotMatchesEverythingOption);

    static QRegularExpression directorsRegEx(R"re(itemprop="director"><a title="([^"]+?)")re");

    MediaElch_Debug_Ensures(titleRegEx.isValid());
    MediaElch_Debug_Ensures(seasonEpisodeRegEx.isValid());
    MediaElch_Debug_Ensures(overviewRegEx.isValid());
    MediaElch_Debug_Ensures(firstAiredRegEx.isValid());
    MediaElch_Debug_Ensures(actorsRegEx.isValid());
    MediaElch_Debug_Ensures(actorNameRegEx.isValid());
    MediaElch_Debug_Ensures(actorRoleRegEx.isValid());
    MediaElch_Debug_Ensures(thumbDivRegEx.isValid());
    MediaElch_Debug_Ensures(thumbRegEx.isValid());
    MediaElch_Debug_Ensures(directorsRegEx.isValid());

    QRegularExpressionMatch seasonEpisodeMatch = seasonEpisodeRegEx.match(html);
    if (seasonEpisodeMatch.hasMatch()) {
        bool ok{false};
        int seasonNumber = seasonEpisodeMatch.captured(1).toInt(&ok);
        if (ok && seasonNumber >= 0) {
            episode().setSeason(SeasonNumber(seasonNumber));
        }
        int episodeNumber = seasonEpisodeMatch.captured(2).toInt(&ok);
        if (ok && episodeNumber >= 0) {
            episode().setEpisode(EpisodeNumber(episodeNumber));
        }
    }

    episode().setTitle(normalizeFromHtml(titleRegEx.match(html).captured(1)));
    episode().setOverview(normalizeFromHtml(overviewRegEx.match(html).captured(1)));
    episode().setFirstAired(QDate::fromString(firstAiredRegEx.match(html).captured(1), "dd.MM.yyyy"));

    // Thumbnail
    QRegularExpressionMatch thumbDivMatch = thumbDivRegEx.match(html);
    if (thumbDivMatch.hasMatch()) {
        QRegularExpressionMatch thumbMatch = thumbRegEx.match(thumbDivMatch.captured(0));
        QUrl thumbUrl(thumbMatch.captured(1));
        if (thumbMatch.hasMatch() && thumbUrl.isValid()) {
            episode().setThumbnail(thumbUrl);
        }
    }

    // Actors
    QRegularExpressionMatchIterator matches = actorsRegEx.globalMatch(html);
    while (matches.hasNext()) {
        const QString actorHtml = matches.next().captured(0);

        // The result is a list of roles.
        QString roles = actorRoleRegEx.match(actorHtml).captured(1);
        normalizeActorRole(roles);

        Actor actor;
        actor.name = normalizeFromHtml(actorNameRegEx.match(actorHtml).captured(1));
        actor.role = normalizeFromHtml(roles);
        QString thumb = actorImageRegEx.match(actorHtml).captured(1);
        if (!thumb.endsWith(".svg")) { // e.g. Person.svg, i.e. placeholder
            actor.thumb = normalizeFromHtml(thumb);
        }

        if (isFernsehserienDeActorRole(actor.role)) {
            episode().addActor(actor);
        }
    }

    // Directors
    matches = directorsRegEx.globalMatch(html);
    QStringList directors;
    while (matches.hasNext()) {
        QString director = normalizeFromHtml(matches.next().captured(1));
        directors.push_back(director);
    }
    if (!directors.isEmpty()) {
        episode().setDirectors(directors);
    }

    // There is also `itemprop="producer"`, but that is not supported by MediaElch, yet.
}

} // namespace scraper
} // namespace mediaelch
