#include "scrapers/tv_show/imdb/ImdbTvEpisodeParser.h"

#include "globals/Helper.h"
#include "globals/Poster.h"
#include "tv_shows/TvDbId.h"
#include "tv_shows/TvShowEpisode.h"

#include <QRegularExpression>
#include <chrono>

namespace mediaelch {
namespace scraper {

void ImdbTvEpisodeParser::parseInfos(TvShowEpisode& episode, const QString& html)
{
    using namespace std::chrono;

    QRegularExpression rx;
    rx.setPatternOptions(QRegularExpression::InvertedGreedinessOption | QRegularExpression::DotMatchesEverythingOption);
    QRegularExpressionMatch match;

    rx.setPattern(R"re(<meta property="pageId" content="(tt\d+)" />)re");
    match = rx.match(html);
    if (match.hasMatch()) {
        episode.setImdbId(ImdbId(match.captured(1).trimmed()));
    }

    rx.setPattern(R"(<h1 class="[^"]*">([^<]*)&nbsp;)");
    match = rx.match(html);
    if (match.hasMatch()) {
        episode.setTitle(match.captured(1).trimmed());
    }
    rx.setPattern(R"(<h1 itemprop="name" class="">(.*)&nbsp;<span id="titleYear">)");
    match = rx.match(html);
    if (match.hasMatch()) {
        episode.setTitle(match.captured(1).trimmed());
    }

    // rx.setPattern(R"(<div class="originalTitle">([^<]*)<span)");
    // match = rx.match(html);
    // if (match.hasMatch()) {
    //     episode.setOriginalName(match.captured(1));
    // }

    // --------------------------------------

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

        QRegularExpressionMatchIterator matches = rx.globalMatch(directorsBlock);
        while (matches.hasNext()) {
            directors << matches.next().captured(1);
        }
        episode.setDirectors(directors);
    }

    // --------------------------------------

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
        QRegularExpressionMatchIterator matches = rx.globalMatch(writersBlock);
        while (matches.hasNext()) {
            writers << matches.next().captured(1).trimmed();
        }
        episode.setWriters(writers);
    }

    // --------------------------------------
    // rx.setPattern(R"(<div class="see-more inline canwrap">\n *<h4 class="inline">Genres:</h4>(.*)</div>)");
    // match = rx.match(html);
    // if (match.hasMatch()) {
    //     QString genres = match.captured(1);
    //     rx.setPattern(R"(<a href="[^"]*"[^>]*>([^<]*)</a>)");
    //     int pos = 0;
    //     while ((pos = rx.indexIn(genres, pos)) != -1) {
    //         episode.addGenre(match.captured(1).trimmed());
    //         pos += rx.matchedLength();
    //     }
    // }

    // --------------------------------------
    // rx.setPattern(R"(<div class="txt-block">[^<]*<h4 class="inline">Taglines:</h4>(.*)</div>)");
    // match = rx.match(html);
    // if (match.hasMatch()) {
    //     QString tagline = match.captured(1);
    //     QRegularExpression rxMore("<span class=\"see-more inline\">.*</span>");
    //     rxMore.setMinimal(true);
    //     tagline.remove(rxMore);
    //     episode.setTagline(tagline.trimmed());
    // }

    // --------------------------------------
    // rx.setPattern(R"(<div class="see-more inline canwrap">\n *<h4 class="inline">Plot Keywords:</h4>(.*)<nobr>)");
    // match = rx.match(html);
    // if (match.hasMatch()) {
    //     QString tags = match.captured(1);
    //     rx.setPattern(R"(<span class="itemprop">([^<]*)</span>)");
    //     int pos = 0;
    //     while ((pos = rx.indexIn(tags, pos)) != -1) {
    //         episode.addTag(match.captured(1).trimmed());
    //         pos += rx.matchedLength();
    //     }
    // }

    // --------------------------------------
    rx.setPattern("<a href=\"[^\"]*\"(.*)title=\"See all release dates\" >[^<]*<meta itemprop=\"datePublished\" "
                  "content=\"([^\"]*)\" />");

    match = rx.match(html);
    if (match.hasMatch()) {
        episode.setFirstAired(QDate::fromString(match.captured(2).trimmed(), "yyyy-MM-dd"));

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
                episode.setFirstAired(QDate(year, month, day));
            }

        } else {
            rx.setPattern(R"(<title>[^<]+(?:\(| )(\d{4})\) - IMDb</title>)");
            match = rx.match(html);
            if (match.hasMatch()) {
                const int day = 1;
                const int month = 1;
                const int year = match.captured(1).trimmed().toInt();
                episode.setFirstAired(QDate(year, month, day));
            }
        }
    }

    // --------------------------------------

    rx.setPattern(R"rx("contentRating": "([^"]*)",)rx");
    match = rx.match(html);
    if (match.hasMatch()) {
        episode.setCertification(Certification(match.captured(1).trimmed()));
    }

    // --------------------------------------
    // rx.setPattern(R"("duration": "PT([0-9]+)H?([0-9]+)M")");
    // if (match.hasMatch()) {
    //     if (match.capturedtureCount() > 1) {
    //         minutes runtime = hours(match.captured(1).toInt()) + minutes(match.captured(2).toInt());
    //         episode.setRuntime(runtime);
    //     } else {
    //         minutes runtime = minutes(match.captured(1).toInt());
    //         episode.setRuntime(runtime);
    //     }
    // }

    // --------------------------------------
    // rx.setPattern(R"(<h4 class="inline">Runtime:</h4>[^<]*<time datetime="PT([0-9]+)M">)");
    // if (match.hasMatch()) {
    //     episode.setRuntime(minutes(match.captured(1).toInt()));
    // }

    // --------------------------------------
    rx.setPattern("<p itemprop=\"description\">(.*)</p>");
    match = rx.match(html);
    if (match.hasMatch()) {
        QString outline = match.captured(1).remove(QRegularExpression("<[^>]*>"));
        outline = outline.remove("See full summary&nbsp;&raquo;").trimmed();
        episode.setOverview(outline);
    }

    // --------------------------------------
    rx.setPattern(R"(<div class="summary_text">(.*)</div>)");
    match = rx.match(html);
    if (match.hasMatch()) {
        QString outline = match.captured(1).remove(QRegularExpression("<[^>]*>"));
        outline = outline.remove("See full summary&nbsp;&raquo;").trimmed();
        episode.setOverview(outline);
    }
    // --------------------------------------

    rx.setPattern(R"(<h2>Storyline</h2>\n +\n +<div class="inline canwrap">\n +<p>\n +<span>(.*)</span>)");
    match = rx.match(html);
    if (match.hasMatch()) {
        QString overview = match.captured(1).trimmed();
        overview.remove(QRegularExpression("<[^>]*>"));
        episode.setOverview(overview.trimmed());
    }
    // --------------------------------------

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

    episode.ratings().setOrAddRating(rating);

    // Top250 for TV shows (used by TheTvDb)
    rx.setPattern(R"re(<link rel='image_src' href="(https://[^"]+.jpg)")re");
    match = rx.match(html);
    if (match.hasMatch()) {
        QString thumbUrlRaw = match.captured(1);
        if (thumbUrlRaw.contains("media-amazon.com")) {
            // Neither the season nor episode page have a proper thumb. But because
            // media-amazon has some auto-crop magic, we can specify the format ourselves.
            // So we use the 16:9 format: 400x225px
            // Most if not all episodes should have thumbs that are bigger than this.
            // This results in the followng postfix:
            const QString imdbThumbSizeSpec = QStringLiteral("._V1_UX400_CR0,0,400,225_AL_.jpg");
            const int index = thumbUrlRaw.lastIndexOf("._V1");
            if (index > -1) {
                thumbUrlRaw.truncate(index);
                thumbUrlRaw.append(imdbThumbSizeSpec);
            }
        }
        episode.setThumbnail(QUrl(thumbUrlRaw));
    }

    // --------------------------------------

    // rx.setPattern(R"(<h4 class="inline">Production Co:</h4>(.*)<span class="see-more inline">)");
    // if (match.hasMatch()) {
    //     QString studios = match.captured(1);
    //     rx.setPattern(R"(<a href="/company/[^"]*"[^>]*>([^<]+)</a>)");
    //     int pos = 0;
    //     while ((pos = rx.indexIn(studios, pos)) != -1) {
    //         episode.setNetwork(match.captured(1).trimmed());
    //         pos += rx.matchedLength();
    //     }
    // }
}

void ImdbTvEpisodeParser::parseIdFromSeason(TvShowEpisode& episode, const QString& html)
{
    QString rawRegex =
        QStringLiteral(R"re(<a href="/title/(tt\d+)/\?ref_=ttep_ep%1")re").arg(episode.episodeNumber().toString());
    QRegularExpression regex(rawRegex);

    QRegularExpressionMatch match = regex.match(html);
    if (!match.hasMatch()) {
        return;
    }

    ImdbId imdbId(match.captured(1).trimmed());
    if (imdbId.isValid()) {
        episode.setImdbId(imdbId);
    }
}

} // namespace scraper
} // namespace mediaelch
