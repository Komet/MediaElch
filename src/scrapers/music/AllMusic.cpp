#include "scrapers/music/AllMusic.h"

#include "music/Album.h"
#include "music/Artist.h"
#include "scrapers/music/UniversalMusicScraper.h"

#include <QRegExp>
#include <QRegularExpression>

namespace mediaelch {
namespace scraper {

AllMusicApi::AllMusicApi(QObject* parent) : QObject(parent)
{
}

QUrl AllMusicApi::makeArtistUrl(const AllMusicId& artistId)
{
    return QUrl(QStringLiteral("https://www.allmusic.com/artist/%1").arg(artistId.toString()));
}

QUrl AllMusicApi::makeArtistBiographyUrl(const AllMusicId& artistId)
{
    return QUrl(QStringLiteral("https://www.allmusic.com/artist/%1/biography").arg(artistId.toString()));
}

AllMusic::AllMusic(QObject* parent) : QObject(parent)
{
}

void AllMusic::parseAndAssignAlbum(const QString& html, Album* album, QSet<MusicScraperInfo> infos)
{
    QRegExp rx;
    rx.setMinimal(true);

    if (UniversalMusicScraper::shouldLoad(MusicScraperInfo::Title, infos, album)) {
        rx.setPattern(R"(<h2 class="album-name" itemprop="name">[\n\s]*(.*)[\n\s]*</h2>)");
        if (rx.indexIn(html) != -1) {
            album->setTitle(trim(rx.cap(1)));
        }
    }

    if (UniversalMusicScraper::shouldLoad(MusicScraperInfo::Artist, infos, album)) {
        rx.setPattern(R"(<h3 class="album-artist"[^>]*>[\n\s]*<span itemprop="name">[\n\s]*<a [^>]*>(.*)</a>)");
        if (rx.indexIn(html) != -1) {
            album->setArtist(trim(rx.cap(1)));
        }
    }

    if (UniversalMusicScraper::shouldLoad(MusicScraperInfo::Review, infos, album)) {
        rx.setPattern(R"(<div class="text" itemprop="reviewBody">(.*)</div>)");
        if (rx.indexIn(html) != -1) {
            QString review = rx.cap(1);
            review.remove(QRegExp("<[^>]*>"));
            album->setReview(trim(review));
        }
    }

    if (UniversalMusicScraper::shouldLoad(MusicScraperInfo::ReleaseDate, infos, album)) {
        rx.setPattern(R"(<h4>[\n\s]*Release Date[\n\s]*</h4>[\n\s]*<span>(.*)</span>)");
        if (rx.indexIn(html) != -1) {
            album->setReleaseDate(trim(rx.cap(1)));
        }
    }

    if (UniversalMusicScraper::shouldLoad(MusicScraperInfo::Rating, infos, album)) {
        rx.setPattern("<div class=\"allmusic-rating rating-allmusic-\\d\" "
                      "itemprop=\"ratingValue\">[\\n\\s]*(\\d)[\\n\\s]*</div>");
        if (rx.indexIn(html) != -1) {
            album->setRating(rx.cap(1).toDouble());
        }
    }

    if (UniversalMusicScraper::shouldLoad(MusicScraperInfo::Year, infos, album)) {
        rx.setPattern(R"(<h4>[\n\s]*Release Date[\n\s]*</h4>[\n\s]*<span>.*([0-9]{4}).*</span>)");
        if (rx.indexIn(html) != -1) {
            album->setYear(rx.cap(1).toInt());
        }
    }

    if (UniversalMusicScraper::shouldLoad(MusicScraperInfo::Genres, infos, album)) {
        rx.setPattern(R"(<h4>[\n\s]*Genre[\n\s]*</h4>[\n\s]*<div>(.*)</div>)");
        if (rx.indexIn(html) != -1) {
            QString genres = rx.cap(1);
            rx.setPattern("<a[^>]*>(.*)</a>");
            int pos = 0;
            while ((pos = rx.indexIn(genres, pos)) != -1) {
                album->addGenre(trim(rx.cap(1)));
                pos += rx.matchedLength();
            }
        }
    }

    if (UniversalMusicScraper::shouldLoad(MusicScraperInfo::Styles, infos, album)) {
        rx.setPattern(R"(<h4>[\n\s]*Styles[\n\s]*</h4>[\n\s]*<div>(.*)</div>)");
        if (rx.indexIn(html) != -1) {
            QString styles = rx.cap(1);
            rx.setPattern("<a [^>]*>([^<]*)</a>");
            int pos = 0;
            while ((pos = rx.indexIn(styles, pos)) != -1) {
                album->addStyle(trim(rx.cap(1)));
                pos += rx.matchedLength();
            }
        }
    }

    if (UniversalMusicScraper::shouldLoad(MusicScraperInfo::Moods, infos, album)) {
        rx.setPattern("<h4>Album Moods</h4>[\\n\\s]*<div>(.*)</div>");
        if (rx.indexIn(html) != -1) {
            QString moods = rx.cap(1);
            rx.setPattern("<a [^>]*>(.*)</a>");
            int pos = 0;
            while ((pos = rx.indexIn(moods, pos)) != -1) {
                album->addMood(trim(rx.cap(1)));
                pos += rx.matchedLength();
            }
        }
    }
}

void AllMusic::parseAndAssignArtist(const QString& html, Artist* artist, QSet<MusicScraperInfo> infos)
{
    QRegExp rx;
    rx.setMinimal(true);

    if (UniversalMusicScraper::shouldLoad(MusicScraperInfo::Name, infos, artist)) {
        rx.setPattern(R"(<h2 class="artist-name" itemprop="name">[\n\s]*(.*)[\n\s]*</h2>)");
        if (rx.indexIn(html) != -1) {
            artist->setName(trim(rx.cap(1)));
        }
    }

    if (UniversalMusicScraper::shouldLoad(MusicScraperInfo::YearsActive, infos, artist)) {
        rx.setPattern("<h4>Active</h4>[\\n\\s]*<div>(.*)</div>");
        if (rx.indexIn(html) != -1) {
            artist->setYearsActive(trim(rx.cap(1)));
        }
    }

    if (UniversalMusicScraper::shouldLoad(MusicScraperInfo::Formed, infos, artist)) {
        rx.setPattern(R"(<h4>[\n\s]*Formed[\n\s]*</h4>[\n\s]*<div>(.*)</div>)");
        if (rx.indexIn(html) != -1) {
            QString formed = rx.cap(1);
            // Remove all HTML elements, e.g. <a href="..."></a>
            formed.remove(QRegularExpression("<.+?>"));
            artist->setFormed(trim(formed));
        }
    }

    if (UniversalMusicScraper::shouldLoad(MusicScraperInfo::Born, infos, artist)) {
        rx.setPattern(R"(<h4>[\n\s]*Born[\n\s]*</h4>[\n\s]*<div>(.*)</div>)");
        if (rx.indexIn(html) != -1) {
            artist->setBorn(trim(rx.cap(1)));
        }
    }

    if (UniversalMusicScraper::shouldLoad(MusicScraperInfo::Died, infos, artist)) {
        rx.setPattern(R"(<h4>[\n\s]*Died[\n\s]*</h4>[\n\s]*<div>(.*)</div>)");
        if (rx.indexIn(html) != -1) {
            artist->setDied(trim(rx.cap(1)));
        }
    }

    if (UniversalMusicScraper::shouldLoad(MusicScraperInfo::Disbanded, infos, artist)) {
        rx.setPattern(R"(<h4>[\n\s]*Disbanded[\n\s]*</h4>[\n\s]*<div>(.*)</div>)");
        if (rx.indexIn(html) != -1) {
            artist->setDisbanded(trim(rx.cap(1)));
        }
    }

    if (UniversalMusicScraper::shouldLoad(MusicScraperInfo::Genres, infos, artist)) {
        rx.setPattern(R"(<h4>[\n\s]*Genre[\n\s]*</h4>[\n\s]*<div>(.*)</div>)");
        if (rx.indexIn(html) != -1) {
            QString genres = rx.cap(1);
            rx.setPattern("<a[^>]*>(.*)</a>");
            int pos = 0;
            while ((pos = rx.indexIn(genres, pos)) != -1) {
                artist->addGenre(trim(rx.cap(1)));
                pos += rx.matchedLength();
            }
        }
    }

    if (UniversalMusicScraper::shouldLoad(MusicScraperInfo::Styles, infos, artist)) {
        rx.setPattern(R"(<h4>[\n\s]*Styles[\n\s]*</h4>[\n\s]*<div>(.*)</div>)");
        if (rx.indexIn(html) != -1) {
            QString styles = rx.cap(1);
            rx.setPattern("<a [^>]*>(.*)</a>");
            int pos = 0;
            while ((pos = rx.indexIn(styles, pos)) != -1) {
                artist->addStyle(trim(rx.cap(1)));
                pos += rx.matchedLength();
            }
        }
    }

    if (UniversalMusicScraper::shouldLoad(MusicScraperInfo::Moods, infos, artist)) {
        rx.setPattern(R"(<h3 class="headline">Artists Moods</h3>[\n\s]*<ul>(.*)</ul>)");
        if (rx.indexIn(html) != -1) {
            QString moods = rx.cap(1);
            rx.setPattern("<a [^>]*>(.*)</a>");
            int pos = 0;
            while ((pos = rx.indexIn(moods, pos)) != -1) {
                artist->addMood(trim(rx.cap(1)));
                pos += rx.matchedLength();
            }
        }
    }
}

void AllMusic::parseAndAssignArtistBiography(const QString& html, Artist* artist, QSet<MusicScraperInfo> infos)
{
    if (UniversalMusicScraper::shouldLoad(MusicScraperInfo::Biography, infos, artist)) {
        QRegExp rx(R"(<div class="text" itemprop="reviewBody">(.*)</div>)");
        rx.setMinimal(true);
        if (rx.indexIn(html) != -1) {
            QString biography = rx.cap(1);
            biography.remove(QRegExp("<[^>]*>"));
            artist->setBiography(trim(biography));
        }
    }
}

void AllMusic::parseAndAssignArtistDiscography(const QString& html, Artist* artist, QSet<MusicScraperInfo> infos)
{
    if (UniversalMusicScraper::shouldLoad(MusicScraperInfo::Discography, infos, artist)) {
        QRegExp rx("<td class=\"year\" data\\-sort\\-value=\"[^\"]*\">[\\n\\s]*(.*)[\\n\\s]*</td>[\\n\\s]*<td "
                   "class=\"title\" data\\-sort\\-value=\"(.*)\">");
        rx.setMinimal(true);
        int pos = 0;
        while ((pos = rx.indexIn(html, pos)) != -1) {
            DiscographyAlbum a;
            a.title = trim(rx.cap(2));
            a.year = trim(rx.cap(1));
            if (!a.title.isEmpty() || !a.year.isEmpty()) {
                artist->addDiscographyAlbum(a);
            }
            pos += rx.matchedLength();
        }
    }
}

QString AllMusic::trim(QString text)
{
    return text.replace(QRegularExpression("\\s\\s+"), " ").trimmed();
}

} // namespace scraper
} // namespace mediaelch
