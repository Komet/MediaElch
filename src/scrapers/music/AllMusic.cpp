#include "scrapers/music/AllMusic.h"

#include "music/Album.h"
#include "music/Artist.h"
#include "scrapers/music/UniversalMusicScraper.h"

#include <QJsonArray>
#include <QJsonDocument>
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
    QRegularExpression rx;
    rx.setPatternOptions(QRegularExpression::DotMatchesEverythingOption | QRegularExpression::InvertedGreedinessOption);
    QRegularExpressionMatch match;

    // We do a HTML + JSON parse
    // Later versions of AllMusic have all their data in a JSON structure.
    rx.setPattern(R"(<script type="application/ld\+json">(.*)</script>)");
    QString json = rx.match(html).captured(1);
    QJsonParseError parseError;
    const auto doc = QJsonDocument::fromJson(json.toUtf8(), &parseError).object();
    const bool useJson = parseError.error == QJsonParseError::NoError;

    if (UniversalMusicScraper::shouldLoad(MusicScraperInfo::Title, infos, album)) {
        rx.setPattern(R"(<h2 class="album-name" itemprop="name">[\n\s]*(.*)[\n\s]*</h2>)");
        match = rx.match(html);
        if (match.hasMatch()) {
            album->setTitle(trim(match.captured(1)));

        } else if (useJson) {
            const QString name = trim(doc.value("name").toString());
            if (!name.isEmpty()) {
                album->setTitle(name);
            }
        }
    }

    if (UniversalMusicScraper::shouldLoad(MusicScraperInfo::Artist, infos, album)) {
        rx.setPattern(R"(<h3 class="album-artist"[^>]*>[\n\s]*<span itemprop="name">[\n\s]*<a [^>]*>(.*)</a>)");
        match = rx.match(html);
        if (match.hasMatch()) {
            album->setArtist(trim(match.captured(1)));

        } else if (useJson) {
            QStringList artists;
            const QJsonArray artistArray = doc.value("byArtist").toArray();
            for (const QJsonValue& value : artistArray) {
                const QString name = trim(value.toObject().value("name").toString());
                if (!name.isEmpty()) {
                    artists.append(name);
                }
            }
            if (!artists.isEmpty()) {
                album->setArtist(artists.join(", "));
            }
        }
    }

    if (UniversalMusicScraper::shouldLoad(MusicScraperInfo::Review, infos, album)) {
        qDebug() << "Test";
        rx.setPattern(R"(<div class="text" itemprop="reviewBody">(.*)</div>)");
        match = rx.match(html);
        if (match.hasMatch()) {
            QString review = match.captured(1);
            review.remove(QRegularExpression("<[^>]*>"));
            album->setReview(trim(review));

        } else if (useJson) {
            const QString review = trim(doc.value("review").toObject().value("reviewBody").toString());
            if (!review.isEmpty()) {
                album->setReview(review);
            }
        }
    }

    if (UniversalMusicScraper::shouldLoad(MusicScraperInfo::ReleaseDate, infos, album)) {
        rx.setPattern(R"(<h4>[\n\s]*Release Date[\n\s]*</h4>[\n\s]*<span>(.*)</span>)");
        match = rx.match(html);
        if (match.hasMatch()) {
            album->setReleaseDate(trim(match.captured(1)));
        }
    }

    if (UniversalMusicScraper::shouldLoad(MusicScraperInfo::Rating, infos, album)) {
        rx.setPattern("<div class=\"allmusic-rating rating-allmusic-\\d\" "
                      "itemprop=\"ratingValue\">[\\n\\s]*(\\d)[\\n\\s]*</div>");
        match = rx.match(html);
        if (match.hasMatch()) {
            album->setRating(match.captured(1).toDouble());
        }
    }

    if (UniversalMusicScraper::shouldLoad(MusicScraperInfo::Year, infos, album)) {
        rx.setPattern(R"(<h4>[\n\s]*Release Date[\n\s]*</h4>[\n\s]*<span>.*([0-9]{4}).*</span>)");
        match = rx.match(html);
        if (match.hasMatch()) {
            album->setYear(match.captured(1).toInt());
        }
    }

    if (UniversalMusicScraper::shouldLoad(MusicScraperInfo::Genres, infos, album)) {
        rx.setPattern(R"(<h4>[\n\s]*Genre[\n\s]*</h4>[\n\s]*<div>(.*)</div>)");
        match = rx.match(html);
        if (match.hasMatch()) {
            QString genres = match.captured(1);
            rx.setPattern("<a[^>]*>(.*)</a>");

            QRegularExpressionMatchIterator matches = rx.globalMatch(genres);
            while (matches.hasNext()) {
                album->addGenre(trim(matches.next().captured(1)));
            }
        }
    }

    if (UniversalMusicScraper::shouldLoad(MusicScraperInfo::Styles, infos, album)) {
        rx.setPattern(R"(<h4>[\n\s]*Styles[\n\s]*</h4>[\n\s]*<div>(.*)</div>)");
        match = rx.match(html);
        if (match.hasMatch()) {
            QString styles = match.captured(1);
            rx.setPattern("<a [^>]*>([^<]*)</a>");

            QRegularExpressionMatchIterator matches = rx.globalMatch(styles);
            while (matches.hasNext()) {
                album->addStyle(trim(matches.next().captured(1)));
            }
        }
    }

    if (UniversalMusicScraper::shouldLoad(MusicScraperInfo::Moods, infos, album)) {
        rx.setPattern("<h4>Album Moods</h4>[\\n\\s]*<div>(.*)</div>");
        match = rx.match(html);
        if (match.hasMatch()) {
            QString moods = match.captured(1);
            rx.setPattern("<a [^>]*>(.*)</a>");

            QRegularExpressionMatchIterator matches = rx.globalMatch(moods);
            while (matches.hasNext()) {
                album->addMood(trim(matches.next().captured(1)));
            }
        }
    }
}

void AllMusic::parseAndAssignArtist(const QString& html, Artist* artist, QSet<MusicScraperInfo> infos)
{
    QRegularExpression rx;
    rx.setPatternOptions(QRegularExpression::DotMatchesEverythingOption | QRegularExpression::InvertedGreedinessOption);
    QRegularExpressionMatch match;

    if (UniversalMusicScraper::shouldLoad(MusicScraperInfo::Name, infos, artist)) {
        rx.setPattern(R"(<h2 class="artist-name" itemprop="name">[\n\s]*(.*)[\n\s]*</h2>)");
        match = rx.match(html);
        if (match.hasMatch()) {
            artist->setName(trim(match.captured(1)));
        }
    }

    if (UniversalMusicScraper::shouldLoad(MusicScraperInfo::YearsActive, infos, artist)) {
        rx.setPattern("<h4>Active</h4>[\\n\\s]*<div>(.*)</div>");
        match = rx.match(html);
        if (match.hasMatch()) {
            artist->setYearsActive(trim(match.captured(1)));
        }
    }

    if (UniversalMusicScraper::shouldLoad(MusicScraperInfo::Formed, infos, artist)) {
        rx.setPattern(R"(<h4>[\n\s]*Formed[\n\s]*</h4>[\n\s]*<div>(.*)</div>)");
        match = rx.match(html);
        if (match.hasMatch()) {
            QString formed = match.captured(1);
            // Remove all HTML elements, e.g. <a href="..."></a>
            formed.remove(QRegularExpression("<.+?>"));
            artist->setFormed(trim(formed));
        }
    }

    if (UniversalMusicScraper::shouldLoad(MusicScraperInfo::Born, infos, artist)) {
        rx.setPattern(R"(<h4>[\n\s]*Born[\n\s]*</h4>[\n\s]*<div>(.*)</div>)");
        match = rx.match(html);
        if (match.hasMatch()) {
            artist->setBorn(trim(match.captured(1)));
        }
    }

    if (UniversalMusicScraper::shouldLoad(MusicScraperInfo::Died, infos, artist)) {
        rx.setPattern(R"(<h4>[\n\s]*Died[\n\s]*</h4>[\n\s]*<div>(.*)</div>)");
        match = rx.match(html);
        if (match.hasMatch()) {
            artist->setDied(trim(match.captured(1)));
        }
    }

    if (UniversalMusicScraper::shouldLoad(MusicScraperInfo::Disbanded, infos, artist)) {
        rx.setPattern(R"(<h4>[\n\s]*Disbanded[\n\s]*</h4>[\n\s]*<div>(.*)</div>)");
        match = rx.match(html);
        if (match.hasMatch()) {
            artist->setDisbanded(trim(match.captured(1)));
        }
    }

    if (UniversalMusicScraper::shouldLoad(MusicScraperInfo::Genres, infos, artist)) {
        rx.setPattern(R"(<h4>[\n\s]*Genre[\n\s]*</h4>[\n\s]*<div>(.*)</div>)");
        match = rx.match(html);
        if (match.hasMatch()) {
            QString genres = match.captured(1);
            rx.setPattern("<a[^>]*>(.*)</a>");

            QRegularExpressionMatchIterator matches = rx.globalMatch(genres);
            while (matches.hasNext()) {
                artist->addGenre(trim(matches.next().captured(1)));
            }
        }
    }

    if (UniversalMusicScraper::shouldLoad(MusicScraperInfo::Styles, infos, artist)) {
        rx.setPattern(R"(<h4>[\n\s]*Styles[\n\s]*</h4>[\n\s]*<div>(.*)</div>)");
        match = rx.match(html);
        if (match.hasMatch()) {
            QString styles = match.captured(1);
            rx.setPattern("<a [^>]*>(.*)</a>");

            QRegularExpressionMatchIterator matches = rx.globalMatch(styles);
            while (matches.hasNext()) {
                artist->addStyle(trim(matches.next().captured(1)));
            }
        }
    }

    if (UniversalMusicScraper::shouldLoad(MusicScraperInfo::Moods, infos, artist)) {
        rx.setPattern(R"(<h3 class="headline">Artists Moods</h3>[\n\s]*<ul>(.*)</ul>)");
        match = rx.match(html);
        if (match.hasMatch()) {
            QString moods = match.captured(1);
            rx.setPattern("<a [^>]*>(.*)</a>");

            QRegularExpressionMatchIterator matches = rx.globalMatch(moods);
            while (matches.hasNext()) {
                artist->addMood(trim(matches.next().captured(1)));
            }
        }
    }
}

void AllMusic::parseAndAssignArtistBiography(const QString& html, Artist* artist, QSet<MusicScraperInfo> infos)
{
    if (UniversalMusicScraper::shouldLoad(MusicScraperInfo::Biography, infos, artist)) {
        QRegularExpression rx(R"(<div class="text" itemprop="reviewBody">(.*)</div>)");
        rx.setPatternOptions(
            QRegularExpression::InvertedGreedinessOption | QRegularExpression::DotMatchesEverythingOption);
        QRegularExpressionMatch match = rx.match(html);
        if (match.hasMatch()) {
            QString biography = match.captured(1);
            biography.remove(QRegularExpression("<[^>]*>"));
            artist->setBiography(trim(biography));
        }
    }
}

void AllMusic::parseAndAssignArtistDiscography(const QString& html, Artist* artist, QSet<MusicScraperInfo> infos)
{
    if (UniversalMusicScraper::shouldLoad(MusicScraperInfo::Discography, infos, artist)) {
        QRegularExpression rx(
            "<td class=\"year\" data\\-sort\\-value=\"[^\"]*\">[\\n\\s]*(.*)[\\n\\s]*</td>[\\n\\s]*<td "
            "class=\"title\" data\\-sort\\-value=\"(.*)\">");
        rx.setPatternOptions(
            QRegularExpression::InvertedGreedinessOption | QRegularExpression::DotMatchesEverythingOption);

        QRegularExpressionMatchIterator matches = rx.globalMatch(html);
        while (matches.hasNext()) {
            QRegularExpressionMatch match = matches.next();

            DiscographyAlbum a;
            a.title = trim(match.captured(2));
            a.year = trim(match.captured(1));
            if (!a.title.isEmpty() || !a.year.isEmpty()) {
                artist->addDiscographyAlbum(a);
            }
        }
    }
}

QString AllMusic::trim(QString text)
{
    return text.replace(QRegularExpression("\\s\\s+"), " ").trimmed();
}

} // namespace scraper
} // namespace mediaelch
