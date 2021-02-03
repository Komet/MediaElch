#include "scrapers/movie/imdb/ImdbMovieSearchJob.h"

#include "scrapers/imdb/ImdbApi.h"

namespace mediaelch {
namespace scraper {

ImdbMovieSearchJob::ImdbMovieSearchJob(ImdbApi& api, MovieSearchJob::Config _config, QObject* parent) :
    MovieSearchJob(_config, parent), m_api{api}
{
}

void ImdbMovieSearchJob::execute()
{
    if (ImdbId::isValidFormat(config().query)) {
        m_api.loadMovie(Locale("en"), ImdbId(config().query), [this](QString data, ScraperError error) {
            if (error.hasError()) {
                m_error = error;
            } else {
                parseIdFromMovieHtml(data);
            }
            emit sigFinished(this);
        });
        return;
    }

    m_api.searchForMovie(Locale("en"), config().query, config().includeAdult, [this](QString data, ScraperError error) {
        if (error.hasError()) {
            m_error = error;
        } else {
            parseSearch(data);
        }
        emit sigFinished(this);
    });
}

void ImdbMovieSearchJob::parseIdFromMovieHtml(const QString& html)
{
    MovieSearchJob::Result result;

    QRegularExpression rx;
    rx.setPatternOptions(QRegularExpression::DotMatchesEverythingOption | QRegularExpression::InvertedGreedinessOption);
    QRegularExpressionMatch match;

    rx.setPattern(R"(<h1 class="header"> <span class="itemprop" itemprop="name">(.*)</span>)");
    match = rx.match(html);
    if (match.hasMatch()) {
        result.title = match.captured(1);

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
            result.title = match.captured(1);
            result.released = QDate::fromString(match.captured(2), "yyyy");
        }
    }

    rx.setPattern(R"(<link rel="canonical" href="https://www.imdb.com/title/(.*)/" />)");
    match = rx.match(html);
    if (match.hasMatch()) {
        result.identifier = MovieIdentifier(match.captured(1));
    }

    if (!result.identifier.str().isEmpty()) {
        m_results << result;
    }
}

void ImdbMovieSearchJob::parseSearch(const QString& html)
{
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
        if (!match.captured(1).isEmpty()) {
            MovieSearchJob::Result result;
            result.title = match.captured(2);
            result.identifier = MovieIdentifier(match.captured(1));
            result.released = QDate::fromString(match.captured(3), "yyyy");
            m_results << result;
        }
    }
}

} // namespace scraper
} // namespace mediaelch
