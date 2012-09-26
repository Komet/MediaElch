#include "CLI.h"
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QRegExp>

#include "globals/DownloadManagerElement.h"
#include "globals/Manager.h"

CLI::CLI(QObject *parent, QStringList arguments) :
    QObject(parent)
{
    m_useFolderName = false;
    m_arguments = arguments;
    m_downloadManager = new DownloadManager(this);
    m_infosToLoad << MovieScraperInfos::Title
                  << MovieScraperInfos::Tagline
                  << MovieScraperInfos::Rating
                  << MovieScraperInfos::Released
                  << MovieScraperInfos::Runtime
                  << MovieScraperInfos::Certification
                  << MovieScraperInfos::Trailer
                  << MovieScraperInfos::Overview
                  << MovieScraperInfos::Poster
                  << MovieScraperInfos::Backdrop
                  << MovieScraperInfos::Actors
                  << MovieScraperInfos::Genres
                  << MovieScraperInfos::Studios
                  << MovieScraperInfos::Countries;

    connect(m_downloadManager, SIGNAL(allDownloadsFinished()), this, SLOT(onDownloadsFinished()));
    connect(m_downloadManager, SIGNAL(downloadFinished(DownloadManagerElement)), this, SLOT(onDownloadFinished(DownloadManagerElement)));
}

void CLI::run()
{
    if (!parseArguments(m_arguments)) {
        emit finished();
        return;
    }

    QFileInfo fi(m_file);
    QString searchTerm = fi.completeBaseName();
    if (!m_searchTerm.isEmpty()) {
        searchTerm = m_searchTerm;
    } else if (m_useFolderName) {
        QStringList path = fi.canonicalPath().split(QDir::separator());
        if (path.count() > 0)
            searchTerm = path.last();
    }

    m_movie = new Movie(QStringList() << m_file, this);
    m_tmdb = new TMDb(this);
    m_tmdb->loadSettings();

    connect(m_tmdb, SIGNAL(searchDone(QList<ScraperSearchResult>)), this, SLOT(onScraperSearchDone(QList<ScraperSearchResult>)));
    connect(m_movie, SIGNAL(loaded(Movie*)), this, SLOT(onScraperLoadDone()));

    if (m_scraperId.isEmpty()) {
        m_tmdb->search(searchTerm);
    } else {
        m_movie->loadData(m_scraperId, m_tmdb, m_infosToLoad);
    }
}

bool CLI::parseArguments(QStringList arguments)
{
    // Remove path to app
    arguments.takeFirst();

    bool searchTermSet = false;

    QRegExp rxFile("^--file=(.*)");
    QRegExp rxFolder("^--use-folder-name$");
    QRegExp rxScraper("^--scraper=(.*)");
    QRegExp rxScraperId("^--scraper-id=(.*)");
    QRegExp rxSearchTerm("^--search-term=(.*)");
    QRegExp rxShowHelp("(^-h$)|(^--help$)");

    foreach (const QString &argument, arguments) {
        if (rxFile.indexIn(argument) != -1) {
            m_file = rxFile.cap(1);
            continue;
        }
        if (rxFolder.indexIn(argument) != -1) {
            m_useFolderName = true;
            continue;
        }
        if (rxScraper.indexIn(argument) != -1) {
            m_scraper = rxScraper.cap(1);
            continue;
        }
        if (rxScraperId.indexIn(argument) != -1) {
            m_scraperId = rxScraperId.cap(1);
            continue;
        }
        if (rxSearchTerm.indexIn(argument) != -1) {
            m_searchTerm = rxSearchTerm.cap(1);
            searchTermSet = true;
            continue;
        }
        if (rxShowHelp.indexIn(argument) != -1) {
            showHelp();
            return false;
        }
        qCritical() << tr("Unknown command line argument \"%1\"").arg(argument);
        showHelp();
        return false;
    }

    if (m_file.isEmpty()) {
        qWarning() << tr("No file given");
        showHelp();
        return false;
    }

    if (m_scraper.isEmpty()) {
        qWarning() << tr("No scraper given");
        showHelp();
        return false;
    }

    if (m_scraper != "tmdb") {
        qWarning() << tr("Unsupported scraper \"%1\"").arg(m_scraper);
        showHelp();
        return false;
    }

    if (searchTermSet && m_searchTerm.isEmpty()) {
        qWarning() << tr("Search term given but empty");
        showHelp();
        return false;
    }

    QFileInfo fi(m_file);
    if (!fi.isFile()) {
        qWarning() << tr("File \"%1\" does not exist").arg(m_file);
        showHelp();
        return false;
    }

    return true;
}

void CLI::onScraperSearchDone(QList<ScraperSearchResult> results)
{
    if (results.count() == 0) {
        qWarning() << "Scraper returned no result";
        emit finished();
        return;
    }
    m_movie->loadData(results.first().id, m_tmdb, m_infosToLoad);
}

void CLI::onScraperLoadDone()
{
    QList<DownloadManagerElement> downloads;
    if (m_movie->posters().count() > 0) {
        DownloadManagerElement d;
        d.imageType = TypePoster;
        d.url = m_movie->posters().at(0).originalUrl;
        downloads.append(d);
    }

    if (m_movie->backdrops().count() > 0) {
        DownloadManagerElement d;
        d.imageType = TypeBackdrop;
        d.url = m_movie->backdrops().at(0).originalUrl;
        downloads.append(d);
    }

    QList<Actor*> actors = m_movie->actorsPointer();
    for (int i=0, n=actors.size() ; i<n ; i++) {
        if (actors.at(i)->thumb.isEmpty())
            continue;
        DownloadManagerElement d;
        d.imageType = TypeActor;
        d.url = QUrl(actors.at(i)->thumb);
        d.actor = actors.at(i);
        downloads.append(d);
    }

    m_downloadManager->setDownloads(downloads);
}

void CLI::onDownloadFinished(DownloadManagerElement elem)
{
    if (elem.imageType == TypePoster)
        m_movie->setPosterImage(elem.image);
    else if (elem.imageType == TypeBackdrop)
        m_movie->setBackdropImage(elem.image);
}

void CLI::onDownloadsFinished()
{
    m_movie->saveData(Manager::instance()->mediaCenterInterface());
    emit finished();
}

void CLI::showHelp()
{
    printf("\nUsage:\n\n");
    printf("MediaElch <args>\n\n");
    printf("  needed arguments:\n");
    printf("    --file=\"/path/to/file.mkv\"     The path to the movie file\n");
    printf("    --scraper=tmdb                 Scraper to use, currently only The Movie Db is supported ;)\n\n");
    printf("  optional arguments:\n");
    printf("    --use-folder-name              Use the folder name for search instead of filename\n");
    printf("    --scraper-id=123               Movie id in the scraper\n");
    printf("    --search-term=\"movie name\"     Use this term instead of filename or foldername\n");
}
