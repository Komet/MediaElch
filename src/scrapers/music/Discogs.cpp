#include "scrapers/music/Discogs.h"

#include "music/Album.h"
#include "music/Artist.h"
#include "scrapers/ScraperUtils.h"
#include "scrapers/music/UniversalMusicScraper.h"

#include <QRegularExpression>

namespace mediaelch {
namespace scraper {

Discogs::Discogs(QObject* parent) : QObject(parent)
{
}

void Discogs::parseAndAssignArtist(const QString& html, Artist* artist, QSet<MusicScraperInfo> infos)
{
    QRegularExpression rx;
    rx.setPatternOptions(QRegularExpression::DotMatchesEverythingOption | QRegularExpression::InvertedGreedinessOption);
    QRegularExpressionMatch match;

    if (UniversalMusicScraper::shouldLoad(MusicScraperInfo::Name, infos, artist)) {
        rx.setPattern(R"(<div class="body">[\n\s]*<h1 class="hide_desktop">(.*)</h1>)");
        match = rx.match(html);
        if (match.hasMatch()) {
            artist->setName(removeHtmlEntities(match.captured(1)));
        }
    }

    if (UniversalMusicScraper::shouldLoad(MusicScraperInfo::Biography, infos, artist)) {
        rx.setPattern(R"(<div [^>]* id="profile">[\n\s]*(.*)[\n\s]*</div>)");
        match = rx.match(html);
        if (match.hasMatch()) {
            artist->setBiography(removeHtmlEntities(match.captured(1)));
        }
    }

    if (UniversalMusicScraper::shouldLoad(MusicScraperInfo::Discography, infos, artist)) {
        rx.setPattern("<table [^>]* id=\"artist\">(.*)</table>");
        match = rx.match(html);
        if (match.hasMatch()) {
            QString table = match.captured(1);
            rx.setPattern(R"(<tr[^>]*data\-object\-id="[^"]*"[^>]*>(.*)</tr>)");
            QRegularExpressionMatchIterator matches = rx.globalMatch(table);
            while (matches.hasNext()) {
                match = matches.next();
                QString str = match.captured(1);

                QRegularExpression rx2(R"(<td class="title"[^>]*>.*<a href="[^"]*">(.*)</a>.*</td>)");
                rx2.setPatternOptions(
                    QRegularExpression::InvertedGreedinessOption | QRegularExpression::DotMatchesEverythingOption);

                DiscographyAlbum a;
                match = rx2.match(str);
                if (match.hasMatch()) {
                    a.title = removeHtmlEntities(match.captured(1));
                }

                // data-header may be "Year: " in the US or "Jahr: " in Germany.
                rx2.setPattern(R"(<td class="year has_header" data\-header="[A-Za-z]+: ">(.*)</td>)");
                match = rx2.match(str);
                if (match.hasMatch()) {
                    a.year = removeHtmlEntities(match.captured(1));
                }

                if (a.title != "" || a.year != "") {
                    artist->addDiscographyAlbum(a);
                }
            }
        }
    }
}

void Discogs::parseAndAssignAlbum(const QString& html, Album* album, QSet<MusicScraperInfo> infos)
{
    QRegularExpression rx;
    rx.setPatternOptions(QRegularExpression::InvertedGreedinessOption | QRegularExpression::DotMatchesEverythingOption);
    QRegularExpressionMatch match;

    if (UniversalMusicScraper::shouldLoad(MusicScraperInfo::Artist, infos, album)) {
        rx.setPattern("<span itemprop=\"byArtist\" itemscope itemtype=\"http://schema.org/MusicGroup\">[\\n\\s]*<span "
                      "itemprop=\"name\" title=\"(.*)\" >");
        match = rx.match(html);
        if (match.hasMatch()) {
            album->setArtist(removeHtmlEntities(match.captured(1)));
        }
    }

    if (UniversalMusicScraper::shouldLoad(MusicScraperInfo::Title, infos, album)) {
        rx.setPattern(R"(<span itemprop="name">[\n\s]*(.*)[\n\s]*</span>)");
        match = rx.match(html);
        if (match.hasMatch()) {
            album->setTitle(removeHtmlEntities(match.captured(1)));
        }
    }

    if (UniversalMusicScraper::shouldLoad(MusicScraperInfo::Genres, infos, album)) {
        rx.setPattern(R"(<div class="content" itemprop="genre">[\n\s]*(.*)[\n\s]*</div>)");
        match = rx.match(html);
        if (match.hasMatch()) {
            QString genres = match.captured(1);
            rx.setPattern(R"(<a href="[^"]*">([^<]*)</a>)");

            QRegularExpressionMatchIterator matches = rx.globalMatch(genres);
            while (matches.hasNext()) {
                album->addGenre(removeHtmlEntities(matches.next().captured(1)));
            }
        }
    }

    if (UniversalMusicScraper::shouldLoad(MusicScraperInfo::Styles, infos, album)) {
        rx.setPattern(R"(<div class="head">Style:</div>[\n\s]*<div class="content">[\n\s]*(.*)[\n\s]*</div>)");
        match = rx.match(html);
        if (match.hasMatch()) {
            QString styles = match.captured(1);
            rx.setPattern(R"(<a href="[^"]*">(.*)</a>)");

            QRegularExpressionMatchIterator matches = rx.globalMatch(styles);
            while (matches.hasNext()) {
                album->addStyle(removeHtmlEntities(matches.next().captured(1)));
            }
        }
    }

    if (UniversalMusicScraper::shouldLoad(MusicScraperInfo::Year, infos, album)) {
        rx.setPattern("<div class=\"head\">Year:</div>[\\n\\s]*<div class=\"content\">[\\n\\s]*<a "
                      "href=\"[^\"]*\">(.*)</a>[\\n\\s]*</div>");
        match = rx.match(html);
        if (match.hasMatch()) {
            bool ok = false;
            const int year = removeHtmlEntities(match.captured(1)).toInt(&ok);
            if (ok && year > 0) {
                album->setYear(year);
            }
        } else {
            // Example: <a href="/de/search/?decade=2000&year=2004">2004</a>
            rx.setPattern(R"(<a href="/[a-z]+/search/\?decade=\d+&year=\d+">(\d+)</a>)");
            match = rx.match(html);
            if (match.hasMatch()) {
                bool ok = false;
                const int year = match.captured(1).toInt(&ok);
                if (ok && year > 0) {
                    album->setYear(year);
                }
            }
        }
    }
}

} // namespace scraper
} // namespace mediaelch
