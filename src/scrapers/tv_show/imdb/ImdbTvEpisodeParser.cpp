#include "scrapers/tv_show/imdb/ImdbTvEpisodeParser.h"

#include "data/Poster.h"
#include "data/TvDbId.h"
#include "data/tv_show/TvShowEpisode.h"
#include "globals/Helper.h"
#include "scrapers/ScraperUtils.h"
#include "scrapers/imdb/ImdbReferencePage.h"

#include <QRegularExpression>
#include <chrono>

namespace mediaelch {
namespace scraper {

void ImdbTvEpisodeParser::parseInfos(TvShowEpisode& episode, const QString& html)
{
    // Note: Expects HTML from https://www.imdb.com/title/tt________/reference
    using namespace std::chrono;

    QRegularExpression rx;
    rx.setPatternOptions(QRegularExpression::InvertedGreedinessOption | QRegularExpression::DotMatchesEverythingOption);
    QRegularExpressionMatch match;

    rx.setPattern(R"re(<meta property="pageId" content="(tt\d+)" />)re");
    match = rx.match(html);
    if (match.hasMatch()) {
        episode.setImdbId(ImdbId(match.captured(1).trimmed()));
    }

    const QString title = ImdbReferencePage::extractTitle(html);
    if (!title.isEmpty()) {
        episode.setTitle(title);
    }

    // Enable once original titles exist for episodes.
    // const QString originalTitle = ImdbReferencePage::extractOriginalTitle(html);
    // if (!originalTitle.isEmpty()) {
    //     episode.setOriginalTitle(originalTitle);
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

    const QDate released = ImdbReferencePage::extractReleaseDate(html);
    if (released.isValid()) {
        episode.setFirstAired(released);
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
    // Overview: Try different formats.

    bool hasOverview = false;
    rx.setPattern(R"re(Plot Summary</td>(.*)</td>)re");
    MediaElch_Debug_Assert(rx.isValid());
    match = rx.match(html);
    if (match.hasMatch()) {
        QString outline = match.captured(1);
        outline = outline.remove("Plot Summary").trimmed();
        outline = outline.remove("Plot Synopsis").trimmed();
        outline = removeHtmlEntities(outline);
        episode.setOverview(outline);
        hasOverview = !outline.isEmpty();
    }

    if (!hasOverview) {
        rx.setPattern(R"(<h2>Storyline</h2>\n +\n +<div class="inline canwrap">\n +<p>\n +<span>(.*)</span>)");
        MediaElch_Debug_Assert(rx.isValid());
        match = rx.match(html);
        if (match.hasMatch()) {
            QString overview = removeHtmlEntities(match.captured(1));
            episode.setOverview(overview);
            hasOverview = !overview.isEmpty();
        }
    }

    if (!hasOverview) {
        rx.setPattern(R"(<section class="titlereference-section-overview">.+<hr>(.+)<hr>)");
        MediaElch_Debug_Assert(rx.isValid());
        match = rx.match(html);
        if (match.hasMatch()) {
            QString overview = removeHtmlEntities(match.captured(1));
            episode.setOverview(overview);
            hasOverview = !overview.isEmpty();
        }
    }

    Q_UNUSED(hasOverview)

    // --------------------------------------

    Rating rating;
    rating.source = "imdb";
    rating.maxRating = 10;
    rx.setPattern("<div class=\"ipl-rating-star ?\">(.*)</div>");
    match = rx.match(html);
    if (match.hasMatch()) {
        QString content = match.captured(1);
        rx.setPattern("<span class=\"ipl-rating-star__rating\">(.*)</span>");
        match = rx.match(content);
        if (match.hasMatch()) {
            rating.rating = match.captured(1).trimmed().replace(",", ".").toDouble();
        }

        rx.setPattern(R"(<span class="ipl-rating-star__total-votes">\((.*)\)</span>)");
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

    if (rating.voteCount > 0 || rating.rating > 0.0) {
        episode.ratings().setOrAddRating(rating);
    }

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
            const elch_ssize_t index = thumbUrlRaw.lastIndexOf("._V1");
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
    // Example JSON:
    //   ```json
    //   {"id":"tt0696611","type":"tvEpisode","season":"2","episode":"0"…}
    //   ```
    QRegularExpression regex(QStringLiteral(R"re("id":"(tt\d+)","type":"tvEpisode","season":"\d+","episode":"%1")re")
                                 .arg(episode.episodeNumber().toString()),
        QRegularExpression::InvertedGreedinessOption | QRegularExpression::DotMatchesEverythingOption);
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
