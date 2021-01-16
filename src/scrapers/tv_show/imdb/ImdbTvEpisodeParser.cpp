#include "scrapers/tv_show/imdb/ImdbTvEpisodeParser.h"

#include "globals/Helper.h"
#include "globals/Poster.h"
#include "tv_shows/TvDbId.h"
#include "tv_shows/TvShowEpisode.h"

#include <QRegExp>
#include <QRegularExpression>
#include <chrono>

namespace mediaelch {
namespace scraper {

void ImdbTvEpisodeParser::parseInfos(TvShowEpisode& episode, const QString& html)
{
    using namespace std::chrono;

    QRegExp rx;
    rx.setMinimal(true);

    rx.setPattern(R"re(<meta property="pageId" content="(tt\d+)" />)re");
    if (rx.indexIn(html) != -1) {
        episode.setImdbId(ImdbId(rx.cap(1).trimmed()));
    }

    rx.setPattern(R"(<h1 class="[^"]*">([^<]*)&nbsp;)");
    if (rx.indexIn(html) != -1) {
        episode.setTitle(rx.cap(1).trimmed());
    }
    rx.setPattern(R"(<h1 itemprop="name" class="">(.*)&nbsp;<span id="titleYear">)");
    if (rx.indexIn(html) != -1) {
        episode.setTitle(rx.cap(1).trimmed());
    }

    // rx.setPattern(R"(<div class="originalTitle">([^<]*)<span)");
    // if (rx.indexIn(html) != -1) {
    //     episode.setOriginalName(rx.cap(1));
    // }

    // --------------------------------------

    rx.setPattern(
        R"(<div class="txt-block" itemprop="director" itemscope itemtype="http://schema.org/Person">(.*)</div>)");
    QString directorsBlock;
    if (rx.indexIn(html) != -1) {
        directorsBlock = rx.cap(1);
    } else {
        // the ghost span may only exist if there are more than 2 directors
        rx.setPattern(
            R"(<div class="credit_summary_item">\n +<h4 class="inline">Directors?:</h4>(.*)(?:<span class="ghost">|</div>))");
        if (rx.indexIn(html) != -1) {
            directorsBlock = rx.cap(1);
        }
    }

    if (!directorsBlock.isEmpty()) {
        QStringList directors;
        rx.setPattern(R"(<a href="[^"]*"[^>]*>([^<]*)</a>)");
        int pos = 0;
        while ((pos = rx.indexIn(directorsBlock, pos)) != -1) {
            directors << rx.cap(1);
            pos += rx.matchedLength();
        }
        episode.setDirectors(directors);
    }

    // --------------------------------------

    rx.setPattern(
        R"(<div class="txt-block" itemprop="creator" itemscope itemtype="http://schema.org/Person">(.*)</div>)");
    QString writersBlock;
    if (rx.indexIn(html) != -1) {
        writersBlock = rx.cap(1);
    } else {
        // the ghost span may only exist if there are more than 2 writers
        rx.setPattern(
            R"(<div class="credit_summary_item">\n +<h4 class="inline">Writers?:</h4>(.*)(?:<span class="ghost">|</div>))");
        if (rx.indexIn(html) != -1) {
            writersBlock = rx.cap(1);
        }
    }

    if (!writersBlock.isEmpty()) {
        QStringList writers;
        rx.setPattern(R"(<a href="[^"]*"[^>]*>([^<]*)</a>)");
        int pos = 0;
        while ((pos = rx.indexIn(writersBlock, pos)) != -1) {
            writers << rx.cap(1).trimmed();
            pos += rx.matchedLength();
        }
        episode.setWriters(writers);
    }

    // --------------------------------------
    // rx.setPattern(R"(<div class="see-more inline canwrap">\n *<h4 class="inline">Genres:</h4>(.*)</div>)");
    // if (rx.indexIn(html) != -1) {
    //     QString genres = rx.cap(1);
    //     rx.setPattern(R"(<a href="[^"]*"[^>]*>([^<]*)</a>)");
    //     int pos = 0;
    //     while ((pos = rx.indexIn(genres, pos)) != -1) {
    //         episode.addGenre(rx.cap(1).trimmed());
    //         pos += rx.matchedLength();
    //     }
    // }

    // --------------------------------------
    // rx.setPattern(R"(<div class="txt-block">[^<]*<h4 class="inline">Taglines:</h4>(.*)</div>)");
    // if (rx.indexIn(html) != -1) {
    //     QString tagline = rx.cap(1);
    //     QRegExp rxMore("<span class=\"see-more inline\">.*</span>");
    //     rxMore.setMinimal(true);
    //     tagline.remove(rxMore);
    //     episode.setTagline(tagline.trimmed());
    // }

    // --------------------------------------
    // rx.setPattern(R"(<div class="see-more inline canwrap">\n *<h4 class="inline">Plot Keywords:</h4>(.*)<nobr>)");
    // if (rx.indexIn(html) != -1) {
    //     QString tags = rx.cap(1);
    //     rx.setPattern(R"(<span class="itemprop">([^<]*)</span>)");
    //     int pos = 0;
    //     while ((pos = rx.indexIn(tags, pos)) != -1) {
    //         episode.addTag(rx.cap(1).trimmed());
    //         pos += rx.matchedLength();
    //     }
    // }

    // --------------------------------------
    rx.setPattern("<a href=\"[^\"]*\"(.*)title=\"See all release dates\" >[^<]*<meta itemprop=\"datePublished\" "
                  "content=\"([^\"]*)\" />");
    if (rx.indexIn(html) != -1) {
        episode.setFirstAired(QDate::fromString(rx.cap(2).trimmed(), "yyyy-MM-dd"));

    } else {
        rx.setPattern(R"(<h4 class="inline">Release Date:</h4> ([0-9]+) ([A-z]*) ([0-9]{4}))");
        if (rx.indexIn(html) != -1) {
            int day = rx.cap(1).trimmed().toInt();
            int month = -1;
            QString monthName = rx.cap(2).trimmed();
            int year = rx.cap(3).trimmed().toInt();
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
            if (rx.indexIn(html) != -1) {
                const int day = 1;
                const int month = 1;
                const int year = rx.cap(1).trimmed().toInt();
                episode.setFirstAired(QDate(year, month, day));
            }
        }
    }

    // --------------------------------------

    rx.setPattern(R"rx("contentRating": "([^"]*)",)rx");
    if (rx.indexIn(html) != -1) {
        episode.setCertification(Certification(rx.cap(1).trimmed()));
    }

    // --------------------------------------
    // rx.setPattern(R"("duration": "PT([0-9]+)H?([0-9]+)M")");
    // if (rx.indexIn(html) != -1) {
    //     if (rx.captureCount() > 1) {
    //         minutes runtime = hours(rx.cap(1).toInt()) + minutes(rx.cap(2).toInt());
    //         episode.setRuntime(runtime);
    //     } else {
    //         minutes runtime = minutes(rx.cap(1).toInt());
    //         episode.setRuntime(runtime);
    //     }
    // }

    // --------------------------------------
    // rx.setPattern(R"(<h4 class="inline">Runtime:</h4>[^<]*<time datetime="PT([0-9]+)M">)");
    // if (rx.indexIn(html) != -1) {
    //     episode.setRuntime(minutes(rx.cap(1).toInt()));
    // }

    // --------------------------------------
    rx.setPattern("<p itemprop=\"description\">(.*)</p>");
    if (rx.indexIn(html) != -1) {
        QString outline = rx.cap(1).remove(QRegExp("<[^>]*>"));
        outline = outline.remove("See full summary&nbsp;&raquo;").trimmed();
        episode.setOverview(outline);
    }

    // --------------------------------------
    rx.setPattern(R"(<div class="summary_text">(.*)</div>)");
    if (rx.indexIn(html) != -1) {
        QString outline = rx.cap(1).remove(QRegExp("<[^>]*>"));
        outline = outline.remove("See full summary&nbsp;&raquo;").trimmed();
        episode.setOverview(outline);
    }
    // --------------------------------------

    rx.setPattern(R"(<h2>Storyline</h2>\n +\n +<div class="inline canwrap">\n +<p>\n +<span>(.*)</span>)");
    if (rx.indexIn(html) != -1) {
        QString overview = rx.cap(1).trimmed();
        overview.remove(QRegExp("<[^>]*>"));
        episode.setOverview(overview.trimmed());
    }
    // --------------------------------------

    Rating rating;
    rating.source = "imdb";
    rating.maxRating = 10;
    rx.setPattern("<div class=\"star-box-details\" itemtype=\"http://schema.org/AggregateRating\" itemscope "
                  "itemprop=\"aggregateRating\">(.*)</div>");
    if (rx.indexIn(html) != -1) {
        QString content = rx.cap(1);
        rx.setPattern("<span itemprop=\"ratingValue\">(.*)</span>");
        if (rx.indexIn(content) != -1) {
            rating.rating = rx.cap(1).trimmed().replace(",", ".").toDouble();
        }

        rx.setPattern("<span itemprop=\"ratingCount\">(.*)</span>");
        if (rx.indexIn(content) != -1) {
            rating.voteCount = rx.cap(1).replace(",", "").replace(".", "").toInt();
        }
    } else {
        rx.setPattern(R"(<div class="imdbRating"[^>]*>\n +<div class="ratingValue">(.*)</div>)");
        if (rx.indexIn(html) != -1) {
            QString content = rx.cap(1);
            rx.setPattern("([0-9]\\.[0-9]) based on ([0-9\\,]*) ");
            if (rx.indexIn(content) != -1) {
                rating.rating = rx.cap(1).trimmed().replace(",", ".").toDouble();
                rating.voteCount = rx.cap(2).replace(",", "").replace(".", "").toInt();
            }
            rx.setPattern("([0-9]\\,[0-9]) based on ([0-9\\.]*) ");
            if (rx.indexIn(content) != -1) {
                rating.rating = rx.cap(1).trimmed().replace(",", ".").toDouble();
                rating.voteCount = rx.cap(2).replace(",", "").replace(".", "").toInt();
            }
        }
    }

    episode.ratings().push_back(rating);

    // Top250 for TV shows (used by TheTvDb)
    rx.setPattern(R"re(<link rel='image_src' href="(https://[^"]+.jpg)")re");
    if (rx.indexIn(html) != -1) {
        QString thumbUrlRaw = rx.cap(1);
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
    // if (rx.indexIn(html) != -1) {
    //     QString studios = rx.cap(1);
    //     rx.setPattern(R"(<a href="/company/[^"]*"[^>]*>([^<]+)</a>)");
    //     int pos = 0;
    //     while ((pos = rx.indexIn(studios, pos)) != -1) {
    //         episode.setNetwork(rx.cap(1).trimmed());
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
