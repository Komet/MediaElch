#include "scrapers/music/Discogs.h"

#include "music/Album.h"
#include "music/Artist.h"
#include "scrapers/music/UniversalMusicScraper.h"

#include <QRegExp>
#include <QRegularExpression>

namespace mediaelch {
namespace scraper {

Discogs::Discogs(QObject* parent) : QObject(parent)
{
}

void Discogs::parseAndAssignArtist(const QString& html, Artist* artist, QSet<MusicScraperInfo> infos)
{
    QRegExp rx;
    rx.setMinimal(true);

    if (UniversalMusicScraper::shouldLoad(MusicScraperInfo::Name, infos, artist)) {
        rx.setPattern(R"(<div class="body">[\n\s]*<h1 class="hide_desktop">(.*)</h1>)");
        if (rx.indexIn(html) != -1) {
            artist->setName(trim(rx.cap(1)));
        }
    }

    if (UniversalMusicScraper::shouldLoad(MusicScraperInfo::Biography, infos, artist)) {
        rx.setPattern(R"(<div [^>]* id="profile">[\n\s]*(.*)[\n\s]*</div>)");
        if (rx.indexIn(html) != -1) {
            artist->setBiography(trim(rx.cap(1)));
        }
    }

    if (UniversalMusicScraper::shouldLoad(MusicScraperInfo::Discography, infos, artist)) {
        rx.setPattern("<table [^>]* id=\"artist\">(.*)</table>");
        if (rx.indexIn(html) != -1) {
            QString table = rx.cap(1);
            rx.setPattern(R"(<tr[^>]*data\-object\-id="[^"]*"[^>]*>(.*)</tr>)");
            int pos = 0;
            while ((pos = rx.indexIn(table, pos)) != -1) {
                QRegExp rx2(R"(<td class="title"[^>]*>.*<a href="[^"]*">(.*)</a>.*</td>)");
                rx2.setMinimal(true);

                DiscographyAlbum a;
                if (rx2.indexIn(rx.cap(1)) != -1) {
                    a.title = trim(rx2.cap(1));
                }

                rx2.setPattern(R"(<td class="year has_header" data\-header="Year: ">(.*)</td>)");
                if (rx2.indexIn(rx.cap(1)) != -1) {
                    a.year = trim(rx2.cap(1));
                }

                if (a.title != "" || a.year != "") {
                    artist->addDiscographyAlbum(a);
                }

                pos += rx.matchedLength();
            }
        }
    }
}

void Discogs::parseAndAssignAlbum(const QString& html, Album* album, QSet<MusicScraperInfo> infos)
{
    QRegExp rx;
    rx.setMinimal(true);

    if (UniversalMusicScraper::shouldLoad(MusicScraperInfo::Artist, infos, album)) {
        rx.setPattern("<span itemprop=\"byArtist\" itemscope itemtype=\"http://schema.org/MusicGroup\">[\\n\\s]*<span "
                      "itemprop=\"name\" title=\"(.*)\" >");
        if (rx.indexIn(html) != -1) {
            album->setArtist(trim(rx.cap(1)));
        }
    }

    if (UniversalMusicScraper::shouldLoad(MusicScraperInfo::Title, infos, album)) {
        rx.setPattern(R"(<span itemprop="name">[\n\s]*(.*)[\n\s]*</span>)");
        if (rx.indexIn(html) != -1) {
            album->setTitle(trim(rx.cap(1)));
        }
    }

    if (UniversalMusicScraper::shouldLoad(MusicScraperInfo::Genres, infos, album)) {
        rx.setPattern(R"(<div class="content" itemprop="genre">[\n\s]*(.*)[\n\s]*</div>)");
        if (rx.indexIn(html) != -1) {
            QString genres = rx.cap(1);
            rx.setPattern(R"(<a href="[^"]*">([^<]*)</a>)");
            int pos = 0;
            while ((pos = rx.indexIn(genres, pos)) != -1) {
                album->addGenre(trim(rx.cap(1)));
                pos += rx.matchedLength();
            }
        }
    }

    if (UniversalMusicScraper::shouldLoad(MusicScraperInfo::Styles, infos, album)) {
        rx.setPattern(R"(<div class="head">Style:</div>[\n\s]*<div class="content">[\n\s]*(.*)[\n\s]*</div>)");
        if (rx.indexIn(html) != -1) {
            QString styles = rx.cap(1);
            rx.setPattern(R"(<a href="[^"]*">(.*)</a>)");
            int pos = 0;
            while ((pos = rx.indexIn(styles, pos)) != -1) {
                album->addStyle(trim(rx.cap(1)));
                pos += rx.matchedLength();
            }
        }
    }

    if (UniversalMusicScraper::shouldLoad(MusicScraperInfo::Year, infos, album)) {
        rx.setPattern("<div class=\"head\">Year:</div>[\\n\\s]*<div class=\"content\">[\\n\\s]*<a "
                      "href=\"[^\"]*\">(.*)</a>[\\n\\s]*</div>");
        if (rx.indexIn(html) != -1) {
            album->setYear(trim(rx.cap(1)).toInt());
        }
    }
}

QString Discogs::trim(QString text)
{
    return text.replace(QRegularExpression("\\s\\s+"), " ").trimmed();
}

} // namespace scraper
} // namespace mediaelch
