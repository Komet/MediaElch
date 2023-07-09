#include "scrapers/music/Discogs.h"

#include "data/music/Album.h"
#include "data/music/Artist.h"
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
        rx.setPattern(R"(<h1[^>]*>(.*)</h1>)");
        match = rx.match(html);
        if (match.hasMatch()) {
            artist->setName(removeHtmlEntities(match.captured(1)));
        }
    }

    if (UniversalMusicScraper::shouldLoad(MusicScraperInfo::Biography, infos, artist)) {
        rx.setPattern(R"(<div class="profileContainer_[^"]+">[\n\s]*(.*)[\n\s]*</div>)");
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
            rx.setPattern(R"(<tr[^>]*class="card [^"]*"[^>]*>(.*)</tr>)");
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
                rx2.setPattern(R"(<td class="year has_header"[^>]+>(.*)</td>)");
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
    // Example: https://www.discogs.com/master/6495-Metallica-Master-Of-Puppets
    QRegularExpression rx;
    rx.setPatternOptions(QRegularExpression::InvertedGreedinessOption | QRegularExpression::DotMatchesEverythingOption);
    QRegularExpressionMatch match;

    rx.setPattern(R"(<h1[^>]+>.*</h1>)");
    match = rx.match(html);
    QString h1 = match.hasMatch() ? match.captured(0) : "";

    if (UniversalMusicScraper::shouldLoad(MusicScraperInfo::Artist, infos, album)) {
        rx.setPattern(R"(<a[^>]+>(.*)</a>)");
        match = rx.match(h1);
        if (match.hasMatch()) {
            album->setArtist(removeHtmlEntities(match.captured(1)));
        }
    }

    if (UniversalMusicScraper::shouldLoad(MusicScraperInfo::Title, infos, album)) {
        rx.setPattern(R"(<!-- -->(.*)</h1>)");
        match = rx.match(h1);
        if (match.hasMatch()) {
            album->setTitle(removeHtmlEntities(match.captured(1)));
        }
    }

    if (UniversalMusicScraper::shouldLoad(MusicScraperInfo::Genres, infos, album)) {
        rx.setPattern(R"(</th><td>(<a href="/(?:[a-z]{2}/)?genre[^>]+>.*)</td>)");
        match = rx.match(html);
        if (match.hasMatch()) {
            QString genres = match.captured(1);
            rx.setPattern(R"(<a href="/(?:[a-z]{2}/)?genre/[^>]+>(.*)</a>)");

            QRegularExpressionMatchIterator matches = rx.globalMatch(genres);
            while (matches.hasNext()) {
                album->addGenre(removeHtmlEntities(matches.next().captured(1)));
            }
        }
    }

    if (UniversalMusicScraper::shouldLoad(MusicScraperInfo::Styles, infos, album)) {
        rx.setPattern(R"(</th><td>(<a href="/(?:[a-z]{2}/)?style[^>]+>.*)</td>)");
        match = rx.match(html);
        if (match.hasMatch()) {
            QString styles = match.captured(1);
            rx.setPattern(R"(<a href="/(?:[a-z]{2}/)?style/[^>]+>(.*)</a>)");

            QRegularExpressionMatchIterator matches = rx.globalMatch(styles);
            while (matches.hasNext()) {
                album->addStyle(removeHtmlEntities(matches.next().captured(1)));
            }
        }
    }

    if (UniversalMusicScraper::shouldLoad(MusicScraperInfo::Year, infos, album)) {
        rx.setPattern(R"(<time dateTime="\d{4}">(\d{4})</time></a></td>)");
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

} // namespace scraper
} // namespace mediaelch
