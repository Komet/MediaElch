#include "scrapers/movie/adultdvdempire/AdultDvdEmpireScrapeJob.h"

#include "movies/Movie.h"
#include "scrapers/movie/adultdvdempire/AdultDvdEmpireApi.h"

#include <QTextDocument>

namespace mediaelch {
namespace scraper {

AdultDvdEmpireScrapeJob::AdultDvdEmpireScrapeJob(AdultDvdEmpireApi& api,
    MovieScrapeJob::Config _config,
    QObject* parent) :
    MovieScrapeJob(std::move(_config), parent), m_api{api}
{
}

void AdultDvdEmpireScrapeJob::execute()
{
    m_api.loadMovie(config().identifier.str(), [this](QString html, ScraperError error) {
        if (!error.hasError()) {
            parseAndAssignInfos(html);

        } else {
            m_error = error;
        }
        emit sigFinished(this);
    });
}


void AdultDvdEmpireScrapeJob::parseAndAssignInfos(const QString& html)
{
    using namespace std::chrono;

    QTextDocument doc;
    QRegularExpression rx;
    rx.setPatternOptions(QRegularExpression::DotMatchesEverythingOption | QRegularExpression::InvertedGreedinessOption);
    QRegularExpressionMatch match;

    rx.setPattern("<h1>(.*)</h1>");
    match = rx.match(html);
    if (match.hasMatch()) {
        doc.setHtml(match.captured(1).trimmed());
        m_movie->setName(doc.toPlainText());
    }

    rx.setPattern("<small>Length: </small> ([0-9]*) hrs. ([0-9]*) mins.[\\s\\n]*</li>");
    match = rx.match(html);
    if (match.hasMatch()) {
        minutes runtime = hours(match.captured(1).toInt()) + minutes(match.captured(2).toInt());
        m_movie->setRuntime(runtime);
    }

    rx.setPattern("<li><small>Production Year:</small> ([0-9]{4})[\\s\\n]*</li>");
    match = rx.match(html);
    if (match.hasMatch()) {
        m_movie->setReleased(QDate::fromString(match.captured(1), "yyyy"));
    }

    rx.setPattern("<li><small>Studio: </small><a href=\"[^\"]*\"[\\s\\n]*Category=\"Item Page\"[\\s\\n]*Label=\"Studio "
                  "- Details\">(.*)[\\s\\n]*</a>");
    match = rx.match(html);
    if (match.hasMatch()) {
        doc.setHtml(match.captured(1));
        m_movie->addStudio(doc.toPlainText().trimmed());
    }

    {
        // clear actors
        m_movie->setActors({});

        QTextDocument text;

        // The Regex is "a bit" more complex because ADE has two HTML styles:
        // One with images and one without. The second Regex line has an OR for this.
        rx.setPattern(
            R"re(<a href="/\d+/[^"]+"\r?\n\s+style="[^"]+"\r?\n\s+Category="Item Page" Label="Performer">)re"
            R"re((?:(?:<div class="[^"]+"><u>([^<]+)</u>(?:<div[^>]+>)*<img src="([^"]+)")|(?:(?:\r?\n\t+)+(.+)</a>)))re");
        rx.optimize();
        QRegularExpressionMatchIterator matches = rx.globalMatch(html);
        while (matches.hasNext()) {
            QRegularExpressionMatch actorMatch = matches.next();
            Actor a;
            if (actorMatch.captured(1).isEmpty()) {
                text.setHtml(actorMatch.captured(3).trimmed());
                a.name = replaceEntities(text.toPlainText());
            } else {
                text.setHtml(actorMatch.captured(1).trimmed());
                a.name = replaceEntities(text.toPlainText());
                a.thumb = actorMatch.captured(2);
            }
            if (!a.name.isEmpty()) {
                m_movie->addActor(a);
            }
        }
    }

    rx.setPattern(R"(<a href="/\d+/[^"]+"\r\n\s+Category="Item Page" Label="Director">([^<]+)</a>)");
    match = rx.match(html);
    if (match.hasMatch()) {
        m_movie->setDirector(match.captured(1).trimmed());
    }

    // get the list of categories first (to avoid parsing categories of other movies)
    rx.setPattern(R"(<strong>Categories:</strong>&nbsp;(.*)</div>)");
    match = rx.match(html);
    if (match.hasMatch()) {
        QString categoryHtml = match.captured(1);
        rx.setPattern(R"(<a href="[^"]*"[\r\s\n]*Category="Item Page" Label="Category">([^<]*)</a>)");

        QRegularExpressionMatchIterator matches = rx.globalMatch(categoryHtml);
        while (matches.hasNext()) {
            m_movie->addGenre(matches.next().captured(1).trimmed());
        }
    }

    rx.setPattern(R"(<h4 class="m-b-0 text-dark synopsis">(<p( class="markdown-h[12]")?>.*)</p></h4>)");
    match = rx.match(html);
    if (match.hasMatch()) {
        // add some newlines to simulate the paragraphs (scene descriptions)
        QString content{match.captured(1).trimmed()};
        content.remove("<p class=\"markdown-h1\">");
        content.remove("<p>");
        content.replace("<p class=\"markdown-h2\">", "<br>");
        content.replace("</p>", "<br>");
        doc.setHtml(content);
        m_movie->setOverview(doc.toPlainText());
    }

    rx.setPattern("href=\"([^\"]*)\"[\\s\\n]*id=\"front-cover\"");
    match = rx.match(html);
    if (match.hasMatch()) {
        Poster p;
        p.thumbUrl = match.captured(1);
        p.originalUrl = match.captured(1);
        m_movie->images().addPoster(p);
    }

    rx.setPattern(R"(<a href="[^"]*"[\s\r\n]*Category="Item Page" Label="Series">[\s\r\n]*([^<]*)<span)");
    match = rx.match(html);
    if (match.hasMatch()) {
        doc.setHtml(match.captured(1));
        QString setName = doc.toPlainText().trimmed();
        if (setName.endsWith("Series", Qt::CaseInsensitive)) {
            setName.chop(6);
        }
        setName = setName.trimmed();
        if (setName.startsWith("\"")) {
            setName.remove(0, 1);
        }
        if (setName.endsWith("\"")) {
            setName.chop(1);
        }
        MovieSet set;
        set.name = setName.trimmed();
        m_movie->setSet(set);
    }

    {
        rx.setPattern(R"re(<a rel="(scene)?screenshots"[\s\n]*href="([^"]*)")re");
        QRegularExpressionMatchIterator matches = rx.globalMatch(html);
        while (matches.hasNext()) {
            QRegularExpressionMatch backDropMatch = matches.next();
            Poster p;
            p.thumbUrl = backDropMatch.captured(2);
            p.originalUrl = backDropMatch.captured(2);
            m_movie->images().addBackdrop(p);
        }
    }
}

QString AdultDvdEmpireScrapeJob::replaceEntities(QString str) const
{
    // Just some common entities that QTextDocument does not replace.
    return str.replace("&#39;", "'");
}

} // namespace scraper
} // namespace mediaelch
