#include "CLI.h"
#include <QApplication>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QRegExp>
#include <QTimer>

#include "globals/DownloadManagerElement.h"
#include "globals/Manager.h"
#include "scrapers/VideoBuster.h"

/**
 * @brief CLI::CLI
 * @param parent
 * @param arguments Command line arguments
 */
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
    Manager::instance();
}

/**
 * @brief Main CLI function.
 */
void CLI::run()
{
    if (!parseArguments(m_arguments)) {
        emit finished();
        return;
    }

    QFileInfo fi(m_movieFile);
    QString searchTerm = fi.completeBaseName();
    if (!m_searchTerm.isEmpty()) {
        searchTerm = m_searchTerm;
    } else if (m_useFolderName) {
        QStringList path = fi.canonicalPath().split(QDir::separator());
        if (path.count() > 0)
            searchTerm = path.last();
    }

    m_movie = new Movie(QStringList() << m_movieFile, this);
    m_scraper = Manager::instance()->getScraperForName(m_scraperName);
    m_scraper->loadSettings();

    connect(m_scraper, SIGNAL(searchDone(QList<ScraperSearchResult>)), this, SLOT(onScraperSearchDone(QList<ScraperSearchResult>)));
    connect(m_movie, SIGNAL(loaded(Movie*)), this, SLOT(onScraperLoadDone()));

    if (m_scraperId.isEmpty()) {
        m_scraper->search(searchTerm);
    } else {
        m_movie->loadData(m_scraperId, m_scraper, m_infosToLoad);
    }
}

/**
 * @brief This function parses the list of arguments and assigns them to local variables.
 *        If an invalid argument was passed this function returns false.
 * @param arguments List of command line arguments
 * @return True if all arguments where ok, false if an error occured
 */
bool CLI::parseArguments(QStringList arguments)
{
    // Remove path to app
    arguments.takeFirst();

    bool searchTermSet = false;

    QRegExp rxMovie("^--movie=(.*)");
    QRegExp rxFolder("^--use-folder-name$");
    QRegExp rxScraper("^--scraper=(.*)");
    QRegExp rxScraperId("^--scraper-id=(.*)");
    QRegExp rxSearchTerm("^--search-term=(.*)");
    QRegExp rxShowHelp("(^-h$)|(^--help$)");

    foreach (const QString &argument, arguments) {
        if (rxMovie.indexIn(argument) != -1) {
            m_movieFile = rxMovie.cap(1);
            continue;
        }
        if (rxFolder.indexIn(argument) != -1) {
            m_useFolderName = true;
            continue;
        }
        if (rxScraper.indexIn(argument) != -1) {
            m_scraperName = rxScraper.cap(1);
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

    if (m_movieFile.isEmpty()) {
        qWarning() << tr("No file given");
        showHelp();
        return false;
    }

    if (m_scraperName.isEmpty()) {
        qWarning() << tr("No scraper given");
        showHelp();
        return false;
    }

    if (m_scraperName != "tmdb" && m_scraperName != "cinefacts" && m_scraperName != "videobuster" && m_scraperName != "ofdb") {
        qWarning() << tr("Unsupported scraper \"%1\"").arg(m_scraperName);
        showHelp();
        return false;
    }

    if (searchTermSet && m_searchTerm.isEmpty()) {
        qWarning() << tr("Search term given but empty");
        showHelp();
        return false;
    }

    QFileInfo fi(m_movieFile);
    if (!fi.isFile()) {
        qWarning() << tr("File \"%1\" does not exist").arg(m_movieFile);
        showHelp();
        return false;
    }

    return true;
}

/**
 * @brief Called when a movie scraper has finished searching.
 *        Starts loading the first result
 * @param results List of Results
 */
void CLI::onScraperSearchDone(QList<ScraperSearchResult> results)
{
    if (results.count() == 0) {
        qWarning() << "Scraper returned no result";
        emit finished();
        return;
    }
    m_movie->loadData(results.first().id, m_scraper, m_infosToLoad);
}

/**
 * @brief Called when a movie scraper has finished loading all data
 *        Starts loading of images for poster, backdrop and actors
 */
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

/**
 * @brief Called when a download has finished.
 *        If the download was a poster or backdrop, the image is assigned to the movie
 * @param elem Download element
 */
void CLI::onDownloadFinished(DownloadManagerElement elem)
{
    if (elem.imageType == TypePoster)
        m_movie->setPosterImage(elem.image);
    else if (elem.imageType == TypeBackdrop)
        m_movie->setBackdropImage(elem.image);
}

/**
 * @brief When all downloads have finished this function tells the movie object to save
 *        and emits the finished signal
 */
void CLI::onDownloadsFinished()
{
    m_movie->saveData(Manager::instance()->mediaCenterInterface());
    emit finished();
}

/**
 * @brief Prints the usage information
 */
void CLI::showHelp()
{
    printf("\nUsage:\n\n");
    printf("MediaElch <args>\n\n");
    printf("  needed arguments:\n");
    printf("    --movie=\"/path/to/file.mkv\"    The path to the movie file\n");
    printf("    --scraper=tmdb                 Scraper to use, one of tmdb, ofdb, cinefacts or videobuster\n\n");
    printf("  optional arguments:\n");
    printf("    --use-folder-name              Use the folder name for search instead of filename\n");
    printf("    --scraper-id=123               Movie id in the scraper\n");
    printf("    --search-term=\"movie name\"     Use this term instead of filename or foldername\n");
}
