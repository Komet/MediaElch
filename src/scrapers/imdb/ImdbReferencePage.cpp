#include "ImdbReferencePage.h"

#include "data/movie/Movie.h"
#include "globals/Helper.h"
#include "scrapers/ScraperUtils.h"

#include <QDate>
#include <QRegularExpression>
#include <QTextDocument>

namespace mediaelch {
namespace scraper {

QString ImdbReferencePage::extractTitle(const QString& html)
{
    QRegularExpression rx;
    rx.setPatternOptions(QRegularExpression::DotMatchesEverythingOption | QRegularExpression::InvertedGreedinessOption);
    QRegularExpressionMatch match;

    rx.setPattern(R"(<h3 itemprop="name">\n([^<]+)<span)");
    match = rx.match(html);
    if (match.hasMatch()) {
        return removeHtmlEntities(match.captured(1).trimmed());
    }
    return {};
}

QString ImdbReferencePage::extractOriginalTitle(const QString& html)
{
    QRegularExpression rx;
    rx.setPatternOptions(QRegularExpression::DotMatchesEverythingOption | QRegularExpression::InvertedGreedinessOption);
    QRegularExpressionMatch match;

    // Original Title
    rx.setPattern(R"(</h3>\n([^\n]+)\n\s+<span class="titlereference-original-title)");
    match = rx.match(html);
    if (match.hasMatch()) {
        return removeHtmlEntities(match.captured(1).trimmed());
    }
    return {};
}

QDate ImdbReferencePage::extractReleaseDate(const QString& html)
{
    QRegularExpression rx;
    rx.setPatternOptions(QRegularExpression::DotMatchesEverythingOption | QRegularExpression::InvertedGreedinessOption);
    QRegularExpressionMatch match;

    rx.setPattern(R"(<a href="/title/tt\d+/releaseinfo">([^<]+)</a>)");
    match = rx.match(html);

    if (match.hasMatch()) {
        // Date format, e.g. "09 Mar 1995 (Germany)"
        rx.setPattern(R"( \(.+\))");
        const QString dateStr = match.captured(1).remove(rx).trimmed();
        // Qt::RFC2822Date is basically "dd MMM yyyy"
        return QDate::fromString(dateStr, Qt::RFC2822Date);
    }
    return {};
}

void ImdbReferencePage::extractStudios(Movie* movie, const QString& html)
{
    QRegularExpression rx(R"(Production Companies</h4>.+<ul class="simpleList">(.+)</ul>)",
        QRegularExpression::DotMatchesEverythingOption | QRegularExpression::InvertedGreedinessOption);

    const QRegularExpressionMatch match = rx.match(html);

    if (match.hasMatch()) {
        QString listHtml = match.captured(1);
        rx.setPattern(R"(<a href="/company/[^"]+">([^<]+)</a>)");
        QRegularExpressionMatchIterator matches = rx.globalMatch(listHtml);

        while (matches.hasNext()) {
            movie->addStudio(helper::mapStudio(removeHtmlEntities(matches.next().captured(1)).trimmed()));
        }
    }
}

void ImdbReferencePage::extractDirectors(Movie* movie, const QString& html)
{
    QRegularExpression rx;
    rx.setPatternOptions(QRegularExpression::DotMatchesEverythingOption | QRegularExpression::InvertedGreedinessOption);
    QRegularExpressionMatch match;

    // Note: Either "Director" or "Directors", depending on their number.
    rx.setPattern(R"re(Directors?:\s?\n\s+<ul class="[^"]+">(.*)</ul>)re");
    match = rx.match(html);
    if (!match.hasMatch()) {
        return;
    }

    QString directorsBlock = match.captured(1);
    QStringList directors;

    rx.setPattern(R"re(href="/name/[^"]+">([^<]+)</a>)re");
    QRegularExpressionMatchIterator matches = rx.globalMatch(directorsBlock);

    while (matches.hasNext()) {
        directors << removeHtmlEntities(matches.next().captured(1)).trimmed();
    }
    movie->setDirector(directors.join(", "));
}

void ImdbReferencePage::extractWriters(Movie* movie, const QString& html)
{
    QRegularExpression rx;
    rx.setPatternOptions(QRegularExpression::DotMatchesEverythingOption | QRegularExpression::InvertedGreedinessOption);
    QRegularExpressionMatch match;

    // Note: Either "Writer" or "Writers", depending on their number.
    rx.setPattern(R"re(Writers?:\s?\n\s+<ul class="[^"]+">(.*)</ul>)re");
    match = rx.match(html);
    if (!match.hasMatch()) {
        return;
    }

    QString writersBlock = match.captured(1);
    QStringList writers;

    rx.setPattern(R"re(href="/name/[^"]+">([^<]+)</a>)re");
    QRegularExpressionMatchIterator matches = rx.globalMatch(writersBlock);

    while (matches.hasNext()) {
        writers << removeHtmlEntities(matches.next().captured(1)).trimmed();
    }
    movie->setWriter(writers.join(", "));
}

void ImdbReferencePage::extractCertification(Movie* movie, const QString& html)
{
    QRegularExpression rx;
    rx.setPatternOptions(QRegularExpression::DotMatchesEverythingOption | QRegularExpression::InvertedGreedinessOption);
    QRegularExpressionMatchIterator matches;

    // TODO: There are also other countries, e.g. DE
    rx.setPattern(R"rx(<a href="/search/title\?certificates=US%3A[^"]+">([^<]+)</a>)rx");
    matches = rx.globalMatch(html);

    QStringList certifications;

    while (matches.hasNext()) {
        QRegularExpressionMatch match = matches.next();
        const QStringList cert = match.captured(1).split(":");
        if (cert.size() == 2) {
            certifications << cert.at(1);
        }
    }

    if (!certifications.isEmpty()) {
        // Some inside note: US has e.g. TV-G and PG. PG is listed last for some reason and I
        // personally prefer it.
        movie->setCertification(helper::mapCertification(Certification(certifications.last())));
    }
}

void ImdbReferencePage::extractGenres(Movie* movie, const QString& html)
{
    QRegularExpression rx;
    rx.setPatternOptions(QRegularExpression::DotMatchesEverythingOption | QRegularExpression::InvertedGreedinessOption);
    QRegularExpressionMatch match;

    rx.setPattern(R"(Genres</td>\n\s+<td>(.+)</td>)");
    match = rx.match(html);

    if (match.hasMatch()) {
        const QString genreHtmlList = match.captured(1);
        rx.setPattern(R"(<a href="/genre/[^"]+">([^<]+)</a>)");
        QRegularExpressionMatchIterator matches = rx.globalMatch(genreHtmlList);

        while (matches.hasNext()) {
            movie->addGenre(helper::mapGenre(matches.next().captured(1).trimmed()));
        }
    }
}

void ImdbReferencePage::extractRating(Movie* movie, const QString& html)
{
    QRegularExpression rx;
    rx.setPatternOptions(QRegularExpression::DotMatchesEverythingOption | QRegularExpression::InvertedGreedinessOption);
    QRegularExpressionMatch match;

    Rating rating;
    rating.source = "imdb";
    rating.maxRating = 10;

    rx.setPattern(R"re(<span class="ipl-rating-star__rating">([0-9.,]+)</span>)re");
    match = rx.match(html);
    if (match.hasMatch()) {
        rating.rating = match.captured(1).trimmed().replace(",", ".").toDouble();
    }
    rx.setPattern(R"re(<span class="ipl-rating-star__total-votes">\(([0-9,.]+)\)</span>)re");
    match = rx.match(html);
    if (match.hasMatch()) {
        rating.voteCount = match.captured(1).trimmed().remove(",").remove(".").toInt();
    }
    if (rating.rating > 0 || rating.voteCount > 0) {
        movie->ratings().setOrAddRating(rating);
    }

    // Top250 for movies
    rx.setPattern("Top Rated Movies:? #([0-9]{1,3})</a>");
    match = rx.match(html);
    if (match.hasMatch()) {
        movie->setTop250(match.captured(1).toInt());
    }
    // Top250 for TV shows (used by TheTvDb)
    rx.setPattern("Top Rated TV:? #([0-9]{1,3})\\n</a>");
    match = rx.match(html);
    if (match.hasMatch()) {
        movie->setTop250(match.captured(1).toInt());
    }
}

void ImdbReferencePage::extractOverview(Movie* movie, const QString& html)
{
    QRegularExpression rx;
    rx.setPatternOptions(QRegularExpression::DotMatchesEverythingOption | QRegularExpression::InvertedGreedinessOption);
    QRegularExpressionMatch match;

    // Outline --------------------------

    rx.setPattern(R"(<section class="titlereference-section-overview">\n\s+<div>(.+)</div>)");
    match = rx.match(html);
    if (match.hasMatch()) {
        const QString outline = match.captured(1).trimmed();
        if (!outline.isEmpty()) {
            movie->setOutline(removeHtmlEntities(outline));
        }
    }

    // Overview --------------------------

    rx.setPattern(R"(Plot Summary</td>\n\s+<td>\n\s+<p>(.+)<)");
    match = rx.match(html);
    if (match.hasMatch()) {
        const QString overview = match.captured(1).trimmed();
        if (!overview.isEmpty()) {
            movie->setOverview(removeHtmlEntities(overview));
        }
    }
}

void ImdbReferencePage::extractTaglines(Movie* movie, const QString& html)
{
    QRegularExpression rx;
    rx.setPatternOptions(QRegularExpression::DotMatchesEverythingOption | QRegularExpression::InvertedGreedinessOption);
    QRegularExpressionMatch match;

    rx.setPattern(R"(Taglines</td>\n\s+<td>(.*)<a href)");
    match = rx.match(html);
    if (match.hasMatch()) {
        const QString tagline = match.captured(1).trimmed();
        if (!tagline.isEmpty()) {
            movie->setTagline(tagline);
        }
    }
}

void ImdbReferencePage::extractTags(Movie* movie, const QString& html)
{
    QRegularExpression rx;
    rx.setPatternOptions(QRegularExpression::DotMatchesEverythingOption | QRegularExpression::InvertedGreedinessOption);
    QRegularExpressionMatch match;

    rx.setPattern(R"(Plot Keywords</td>\n\s+<td>(.*)</ul>)");
    match = rx.match(html);
    if (match.hasMatch()) {
        const QString tagsHtml = match.captured(1);
        rx.setPattern(R"(<a href="/keyword/[^"]+">([^<]+)</a>)");
        QRegularExpressionMatchIterator tagMatches = rx.globalMatch(tagsHtml);

        while (tagMatches.hasNext()) {
            const QString tag = tagMatches.next().captured(1).trimmed();
            if (!tag.isEmpty()) {
                movie->addTag(tag);
            }
        }
    }
}

void ImdbReferencePage::extractCountries(Movie* movie, const QString& html)
{
    QRegularExpression rx;
    rx.setPatternOptions(QRegularExpression::DotMatchesEverythingOption | QRegularExpression::InvertedGreedinessOption);
    QRegularExpressionMatch match;

    rx.setPattern(R"(Country</td>(.*)</ul>)");
    match = rx.match(html);
    if (match.hasMatch()) {
        const QString content = match.captured(1);
        rx.setPattern(R"(<a href="/country/[^"]+">([^<]+)</a>)");
        QRegularExpressionMatchIterator countryMatches = rx.globalMatch(content);
        while (countryMatches.hasNext()) {
            movie->addCountry(helper::mapCountry(countryMatches.next().captured(1).trimmed()));
        }
    }
}

} // namespace scraper
} // namespace mediaelch
