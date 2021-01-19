#include "ImdbMovie.h"

#include <QCheckBox>
#include <QGridLayout>
#include <QWidget>

#include "data/Storage.h"
#include "globals/Helper.h"
#include "scrapers/movie/imdb/ImdbMovieScraper.h"
#include "settings/Settings.h"
#include "ui/main/MainWindow.h"

namespace mediaelch {
namespace scraper {

ImdbMovie::ImdbMovie(QObject* parent) : MovieScraper(parent)
{
    m_meta.identifier = ID;
    m_meta.name = "IMDb";
    m_meta.description = tr("IMDb is the world's most popular and authoritative source for movie, TV "
                            "and celebrity content, designed to help fans explore the world of movies "
                            "and shows and decide what to watch.");
    m_meta.website = "https://www.imdb.com/whats-on-tv/";
    m_meta.termsOfService = "https://www.imdb.com/conditions";
    m_meta.privacyPolicy = "https://www.imdb.com/privacy";
    m_meta.help = "https://help.imdb.com";
    m_meta.supportedDetails = {MovieScraperInfo::Title,
        MovieScraperInfo::Director,
        MovieScraperInfo::Writer,
        MovieScraperInfo::Genres,
        MovieScraperInfo::Tags,
        MovieScraperInfo::Released,
        MovieScraperInfo::Certification,
        MovieScraperInfo::Runtime,
        MovieScraperInfo::Overview,
        MovieScraperInfo::Rating,
        MovieScraperInfo::Tagline,
        MovieScraperInfo::Studios,
        MovieScraperInfo::Countries,
        MovieScraperInfo::Actors,
        MovieScraperInfo::Poster};
    m_meta.supportedLanguages = {"en"};
    m_meta.defaultLocale = "en";
    m_meta.isAdult = false;

    m_settingsWidget = new QWidget(MainWindow::instance());
    m_loadAllTagsWidget = new QCheckBox(tr("Load all tags"), m_settingsWidget);
    auto* layout = new QGridLayout(m_settingsWidget);
    layout->addWidget(m_loadAllTagsWidget, 0, 0);
    layout->setContentsMargins(12, 0, 12, 12);
    m_settingsWidget->setLayout(layout);
}

const MovieScraper::ScraperMeta& ImdbMovie::meta() const
{
    return m_meta;
}

void ImdbMovie::initialize()
{
    // no-op
    // IMDb requires no initialization.
}

bool ImdbMovie::isInitialized() const
{
    // IMDb requires no initialization.
    return true;
}

bool ImdbMovie::hasSettings() const
{
    return true;
}

QWidget* ImdbMovie::settingsWidget()
{
    return m_settingsWidget;
}

void ImdbMovie::loadSettings(ScraperSettings& settings)
{
    m_loadAllTags = settings.valueBool("LoadAllTags", false);
    m_loadAllTagsWidget->setChecked(m_loadAllTags);
}

void ImdbMovie::saveSettings(ScraperSettings& settings)
{
    m_loadAllTags = m_loadAllTagsWidget->isChecked();
    settings.setBool("LoadAllTags", m_loadAllTags);
}

QSet<MovieScraperInfo> ImdbMovie::scraperNativelySupports()
{
    return m_meta.supportedDetails;
}

void ImdbMovie::changeLanguage(mediaelch::Locale /*locale*/)
{
    // no-op: Only one language is supported and it is hard-coded.
}

void ImdbMovie::search(QString searchStr)
{
    if (ImdbId::isValidFormat(searchStr)) {
        m_api.loadMovie(Locale("en"), ImdbId(searchStr), [this](QString data, ScraperError error) {
            if (error.hasError()) {
                qWarning() << "[IMDb] Search Error" << error.message << "|" << error.technical;
                emit searchDone({}, error);
                return;
            }

            ScraperSearchResult result = parseIdFromMovieHtml(data);
            if (!result.id.isEmpty()) {
                emit searchDone({result}, {});
            }
        });

    } else {
        m_api.searchForMovie(Locale("en"),
            searchStr,
            Settings::instance()->showAdultScrapers(),
            [this](QString data, ScraperError error) {
                if (error.hasError()) {
                    qWarning() << "[IMDb] Search Error" << error.message << "|" << error.technical;
                    emit searchDone({}, error);
                    return;
                }

                emit searchDone(parseSearch(data), {});
            });
    }
}

ScraperSearchResult ImdbMovie::parseIdFromMovieHtml(const QString& html)
{
    ScraperSearchResult result;

    QRegularExpression rx;
    rx.setPatternOptions(QRegularExpression::DotMatchesEverythingOption | QRegularExpression::InvertedGreedinessOption);
    QRegularExpressionMatch match;

    rx.setPattern(R"(<h1 class="header"> <span class="itemprop" itemprop="name">(.*)</span>)");
    match = rx.match(html);
    if (match.hasMatch()) {
        result.name = match.captured(1);

        rx.setPattern("<h1 class=\"header\"> <span class=\"itemprop\" itemprop=\"name\">.*<span "
                      "class=\"nobr\">\\(<a href=\"[^\"]*\" >([0-9]*)</a>\\)</span>");
        match = rx.match(html);
        if (match.hasMatch()) {
            result.released = QDate::fromString(match.captured(1), "yyyy");

        } else {
            rx.setPattern("<h1 class=\"header\"> <span class=\"itemprop\" itemprop=\"name\">.*</span>.*<span "
                          "class=\"nobr\">\\(([0-9]*)\\)</span>");
            match = rx.match(html);
            if (match.hasMatch()) {
                result.released = QDate::fromString(match.captured(1), "yyyy");
            }
        }
    } else {
        rx.setPattern(R"(<h1 class="">(.*)&nbsp;<span id="titleYear">\(<a href="/year/([0-9]+)/\?ref_=tt_ov_inf")");
        match = rx.match(html);
        if (match.hasMatch()) {
            result.name = match.captured(1);
            result.released = QDate::fromString(match.captured(2), "yyyy");
        }
    }

    rx.setPattern(R"(<link rel="canonical" href="https://www.imdb.com/title/(.*)/" />)");
    match = rx.match(html);
    if (match.hasMatch()) {
        result.id = match.captured(1);
    }

    return result;
}

QVector<ScraperSearchResult> ImdbMovie::parseSearch(const QString& html)
{
    QVector<ScraperSearchResult> results;
    QRegularExpression rx;

    if (html.contains("Including Adult Titles")) {
        // Search result table from "https://www.imdb.com/search/title/?title=..."
        rx.setPattern(R"(<a href="/title/(tt[\d]+)/[^"]*"\n>([^<]*)</a>\n\s*<span[^>]*>\((\d+)\)</span>)");
    } else {
        // Search result table from "https://www.imdb.com/find?q=..."
        rx.setPattern("<td class=\"result_text\"> <a href=\"/title/([t]*[\\d]+)/[^\"]*\" >([^<]*)</a>(?: \\(I+\\)"
                      ")? \\(([0-9]+)\\) (?:</td>|<br/>)");
    }

    rx.setPatternOptions(QRegularExpression::DotMatchesEverythingOption | QRegularExpression::InvertedGreedinessOption);
    QRegularExpressionMatchIterator matches = rx.globalMatch(html);

    while (matches.hasNext()) {
        QRegularExpressionMatch match = matches.next();
        ScraperSearchResult result;
        result.name = match.captured(2);
        result.id = match.captured(1);
        result.released = QDate::fromString(match.captured(3), "yyyy");
        results.append(result);
    }
    return results;
}

void ImdbMovie::loadData(QHash<MovieScraper*, QString> ids, Movie* movie, QSet<MovieScraperInfo> infos)
{
    if (movie == nullptr) {
        return;
    }
    ImdbId imdbId(ids.values().first());
    auto* loader =
        new mediaelch::scraper::ImdbMovieLoader(m_api, *this, imdbId, *movie, std::move(infos), m_loadAllTags, this);
    connect(loader, &ImdbMovieLoader::sigLoadDone, this, &ImdbMovie::onLoadDone);
    loader->load();
}

void ImdbMovie::onLoadDone(Movie& movie, mediaelch::scraper::ImdbMovieLoader* loader)
{
    loader->deleteLater();
    movie.controller()->scraperLoadDone(this);
}

void ImdbMovie::parseAndAssignInfos(const QString& html, Movie* movie, QSet<MovieScraperInfo> infos) const
{
    using namespace std::chrono;

    QRegularExpression rx;
    rx.setPatternOptions(QRegularExpression::DotMatchesEverythingOption | QRegularExpression::InvertedGreedinessOption);
    QRegularExpressionMatch match;

    if (infos.contains(MovieScraperInfo::Title)) {
        rx.setPattern(R"(<h1 class="[^"]*">([^<]*)&nbsp;)");
        match = rx.match(html);
        if (match.hasMatch()) {
            movie->setName(match.captured(1));
        }
        rx.setPattern(R"(<h1 itemprop="name" class="">(.*)&nbsp;<span id="titleYear">)");
        match = rx.match(html);
        if (match.hasMatch()) {
            movie->setName(match.captured(1));
        }
        rx.setPattern(R"(<div class="originalTitle">([^<]*)<span)");
        match = rx.match(html);
        if (match.hasMatch()) {
            movie->setOriginalName(match.captured(1));
        }
    }

    if (infos.contains(MovieScraperInfo::Director)) {
        rx.setPattern(
            R"(<div class="txt-block" itemprop="director" itemscope itemtype="http://schema.org/Person">(.*)</div>)");
        match = rx.match(html);
        QString directorsBlock;
        if (match.hasMatch()) {
            directorsBlock = match.captured(1);
        } else {
            // the ghost span may only exist if there are more than 2 directors
            rx.setPattern(
                R"(<div class="credit_summary_item">\n +<h4 class="inline">Directors?:</h4>(.*)(?:<span class="ghost">|</div>))");
            match = rx.match(html);
            if (match.hasMatch()) {
                directorsBlock = match.captured(1);
            }
        }

        if (!directorsBlock.isEmpty()) {
            QStringList directors;
            rx.setPattern(R"(<a href="[^"]*"[^>]*>([^<]*)</a>)");
            QRegularExpressionMatchIterator directorMatches = rx.globalMatch(directorsBlock);
            while (directorMatches.hasNext()) {
                directors << directorMatches.next().captured(1);
            }
            movie->setDirector(directors.join(", "));
        }
    }

    if (infos.contains(MovieScraperInfo::Writer)) {
        rx.setPattern(
            R"(<div class="txt-block" itemprop="creator" itemscope itemtype="http://schema.org/Person">(.*)</div>)");
        match = rx.match(html);
        QString writersBlock;
        if (match.hasMatch()) {
            writersBlock = match.captured(1);
        } else {
            // the ghost span may only exist if there are more than 2 writers
            rx.setPattern(
                R"(<div class="credit_summary_item">\n +<h4 class="inline">Writers?:</h4>(.*)(?:<span class="ghost">|</div>))");
            match = rx.match(html);
            if (match.hasMatch()) {
                writersBlock = match.captured(1);
            }
        }

        if (!writersBlock.isEmpty()) {
            QStringList writers;
            rx.setPattern(R"(<a href="[^"]*"[^>]*>([^<]*)</a>)");
            QRegularExpressionMatchIterator writerMatches = rx.globalMatch(writersBlock);
            while (writerMatches.hasNext()) {
                writers << writerMatches.next().captured(1);
            }
            movie->setWriter(writers.join(", "));
        }
    }

    rx.setPattern(R"(<div class="see-more inline canwrap">\n *<h4 class="inline">Genres:</h4>(.*)</div>)");
    match = rx.match(html);
    if (infos.contains(MovieScraperInfo::Genres) && match.hasMatch()) {
        QString genres = match.captured(1);
        rx.setPattern(R"(<a href="[^"]*"[^>]*>([^<]*)</a>)");
        QRegularExpressionMatchIterator genreMatches = rx.globalMatch(genres);
        while (genreMatches.hasNext()) {
            movie->addGenre(helper::mapGenre(genreMatches.next().captured(1).trimmed()));
        }
    }

    rx.setPattern(R"(<div class="txt-block">[^<]*<h4 class="inline">Taglines:</h4>(.*)</div>)");
    match = rx.match(html);
    if (infos.contains(MovieScraperInfo::Tagline) && match.hasMatch()) {
        rx.setPattern("<span class=\"see-more inline\">.*</span>");
        const QString tagline = match.captured(1).remove(rx).trimmed();
        movie->setTagline(tagline);
    }

    rx.setPattern(R"(<div class="see-more inline canwrap">\n *<h4 class="inline">Plot Keywords:</h4>(.*)<nobr>)");
    match = rx.match(html);
    if (!m_loadAllTags && infos.contains(MovieScraperInfo::Tags) && match.hasMatch()) {
        QString tags = match.captured(1);
        rx.setPattern(R"(<span class="itemprop">([^<]*)</span>)");
        QRegularExpressionMatchIterator tagMatches = rx.globalMatch(tags);
        while (tagMatches.hasNext()) {
            movie->addTag(tagMatches.next().captured(1).trimmed());
        }
    }

    if (infos.contains(MovieScraperInfo::Released)) {
        rx.setPattern("<a href=\"[^\"]*\"(.*)title=\"See all release dates\" >[^<]*<meta itemprop=\"datePublished\" "
                      "content=\"([^\"]*)\" />");
        match = rx.match(html);
        if (match.hasMatch()) {
            movie->setReleased(QDate::fromString(match.captured(2), "yyyy-MM-dd"));

        } else {
            rx.setPattern(R"(<h4 class="inline">Release Date:</h4> ([0-9]+) ([A-z]*) ([0-9]{4}))");
            match = rx.match(html);
            if (match.hasMatch()) {
                int day = match.captured(1).trimmed().toInt();
                int month = -1;
                QString monthName = match.captured(2).trimmed();
                int year = match.captured(3).trimmed().toInt();
                if (monthName.contains("January", Qt::CaseInsensitive)) {
                    month = 1;
                } else if (monthName.contains("February", Qt::CaseInsensitive)) {
                    month = 2;
                } else if (monthName.contains("March", Qt::CaseInsensitive)) {
                    month = 3;
                } else if (monthName.contains("April", Qt::CaseInsensitive)) {
                    month = 4;
                } else if (monthName.contains("May", Qt::CaseInsensitive)) {
                    month = 5;
                } else if (monthName.contains("June", Qt::CaseInsensitive)) {
                    month = 6;
                } else if (monthName.contains("July", Qt::CaseInsensitive)) {
                    month = 7;
                } else if (monthName.contains("August", Qt::CaseInsensitive)) {
                    month = 8;
                } else if (monthName.contains("September", Qt::CaseInsensitive)) {
                    month = 9;
                } else if (monthName.contains("October", Qt::CaseInsensitive)) {
                    month = 10;
                } else if (monthName.contains("November", Qt::CaseInsensitive)) {
                    month = 11;
                } else if (monthName.contains("December", Qt::CaseInsensitive)) {
                    month = 12;
                }

                if (day != 0 && month != -1 && year != 0) {
                    movie->setReleased(QDate(year, month, day));
                }

            } else {
                rx.setPattern(R"(<title>[^<]+(?:\(| )(\d{4})\) - IMDb</title>)");
                match = rx.match(html);
                if (match.hasMatch()) {
                    const int day = 1;
                    const int month = 1;
                    const int year = match.captured(1).trimmed().toInt();
                    movie->setReleased(QDate(year, month, day));
                }
            }
        }
    }

    rx.setPattern(R"rx("contentRating": "([^"]*)",)rx");
    match = rx.match(html);
    if (infos.contains(MovieScraperInfo::Certification) && match.hasMatch()) {
        movie->setCertification(helper::mapCertification(Certification(match.captured(1))));
    }

    rx.setPattern(R"("duration": "PT([0-9]+)H?([0-9]+)M")");
    match = rx.match(html);
    if (infos.contains(MovieScraperInfo::Runtime) && match.hasMatch()) {
        if (rx.captureCount() > 1) {
            minutes runtime = hours(match.captured(1).toInt()) + minutes(match.captured(2).toInt());
            movie->setRuntime(runtime);
        } else {
            minutes runtime = minutes(match.captured(1).toInt());
            movie->setRuntime(runtime);
        }
    }

    rx.setPattern(R"(<h4 class="inline">Runtime:</h4>[^<]*<time datetime="PT([0-9]+)M">)");
    match = rx.match(html);
    if (infos.contains(MovieScraperInfo::Runtime) && match.hasMatch()) {
        movie->setRuntime(minutes(match.captured(1).toInt()));
    }

    rx.setPattern("<p itemprop=\"description\">(.*)</p>");
    match = rx.match(html);
    if (infos.contains(MovieScraperInfo::Overview) && match.hasMatch()) {
        QString outline = match.captured(1).remove(QRegularExpression("<[^>]*>"));
        outline = outline.remove("See full summary&nbsp;&raquo;").trimmed();
        movie->setOutline(outline);
    }

    rx.setPattern(R"(<div class="summary_text">(.*)</div>)");
    match = rx.match(html);
    if (infos.contains(MovieScraperInfo::Overview) && match.hasMatch()) {
        QString outline = match.captured(1).remove(QRegularExpression("<[^>]*>"));
        outline = outline.remove("See full summary&nbsp;&raquo;").trimmed();
        movie->setOutline(outline);
    }

    rx.setPattern(R"(<h2>Storyline</h2>\n +\n +<div class="inline canwrap">\n +<p>\n +<span>(.*)</span>)");
    match = rx.match(html);
    if (infos.contains(MovieScraperInfo::Overview) && match.hasMatch()) {
        QString overview = match.captured(1).trimmed();
        overview.remove(QRegularExpression("<[^>]*>"));
        movie->setOverview(overview.trimmed());
    }

    if (infos.contains(MovieScraperInfo::Rating)) {
        Rating rating;
        rating.source = "imdb";
        rating.maxRating = 10;
        rx.setPattern("<div class=\"star-box-details\" itemtype=\"http://schema.org/AggregateRating\" itemscope "
                      "itemprop=\"aggregateRating\">(.*)</div>");
        match = rx.match(html);
        if (match.hasMatch()) {
            QString content = match.captured(1);
            rx.setPattern("<span itemprop=\"ratingValue\">(.*)</span>");
            match = rx.match(content);
            if (match.hasMatch()) {
                rating.rating = match.captured(1).trimmed().replace(",", ".").toDouble();
            }

            rx.setPattern("<span itemprop=\"ratingCount\">(.*)</span>");
            match = rx.match(content);
            if (match.hasMatch()) {
                rating.voteCount = match.captured(1).replace(",", "").replace(".", "").toInt();
            }
        } else {
            rx.setPattern(R"(<div class="imdbRating"[^>]*>\n +<div class="ratingValue">(.*)</div>)");
            match = rx.match(html);
            if (match.hasMatch()) {
                QString content = match.captured(1);
                rx.setPattern("([0-9]\\.[0-9]) based on ([0-9\\,]*) ");
                match = rx.match(content);
                if (match.hasMatch()) {
                    rating.rating = match.captured(1).trimmed().replace(",", ".").toDouble();
                    rating.voteCount = match.captured(2).replace(",", "").replace(".", "").toInt();
                }
                rx.setPattern("([0-9]\\,[0-9]) based on ([0-9\\.]*) ");
                match = rx.match(content);
                if (match.hasMatch()) {
                    rating.rating = match.captured(1).trimmed().replace(",", ".").toDouble();
                    rating.voteCount = match.captured(2).replace(",", "").replace(".", "").toInt();
                }
            }
        }

        movie->ratings().push_back(rating);

        // Top250 for movies
        rx.setPattern("Top Rated Movies #([0-9]+)\\n</a>");
        match = rx.match(html);
        if (match.hasMatch()) {
            movie->setTop250(match.captured(1).toInt());
        }
        // Top250 for TV shows (used by TheTvDb)
        rx.setPattern("Top Rated TV #([0-9]+)\\n</a>");
        match = rx.match(html);
        if (match.hasMatch()) {
            movie->setTop250(match.captured(1).toInt());
        }
    }

    rx.setPattern(R"(<h4 class="inline">Production Co:</h4>(.*)<span class="see-more inline">)");
    match = rx.match(html);
    if (infos.contains(MovieScraperInfo::Studios) && match.hasMatch()) {
        QString studios = match.captured(1);
        rx.setPattern(R"(<a href="/company/[^"]*"[^>]*>([^<]+)</a>)");
        QRegularExpressionMatchIterator studioMatches = rx.globalMatch(studios);
        while (studioMatches.hasNext()) {
            movie->addStudio(helper::mapStudio(studioMatches.next().captured(1).trimmed()));
        }
    }

    rx.setPattern(R"(<h4 class="inline">Country:</h4>(.*)</div>)");
    match = rx.match(html);
    if (infos.contains(MovieScraperInfo::Countries) && match.hasMatch()) {
        QString content = match.captured(1);
        rx.setPattern(R"(<a href="[^"]*"[^>]*>([^<]*)</a>)");
        QRegularExpressionMatchIterator countryMatches = rx.globalMatch(content);
        while (countryMatches.hasNext()) {
            movie->addCountry(helper::mapCountry(countryMatches.next().captured(1).trimmed()));
        }
    }
}

} // namespace scraper
} // namespace mediaelch
