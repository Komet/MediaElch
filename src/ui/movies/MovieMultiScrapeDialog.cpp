#include "MovieMultiScrapeDialog.h"
#include "ui_MovieMultiScrapeDialog.h"

#include "globals/Manager.h"
#include "scrapers/movie/CustomMovieScraper.h"
#include "scrapers/movie/IMDB.h"
#include "scrapers/movie/TMDb.h"
#include "settings/Settings.h"
#include "ui/small_widgets/MyCheckBox.h"

MovieMultiScrapeDialog::MovieMultiScrapeDialog(QWidget* parent) : QDialog(parent), ui(new Ui::MovieMultiScrapeDialog)
{
    ui->setupUi(this);

#ifdef Q_OS_MAC
    setWindowFlags((windowFlags() & ~Qt::WindowType_Mask) | Qt::Sheet);
#else
    setWindowFlags((windowFlags() & ~Qt::WindowType_Mask) | Qt::Dialog);
#endif

    QFont font = ui->movieCounter->font();
#ifdef Q_OS_WIN
    font.setPointSize(font.pointSize() - 1);
#else
    font.setPointSize(font.pointSize() - 2);
#endif
    ui->movieCounter->setFont(font);

    m_executed = false;
    m_currentMovie = nullptr;

    ui->chkActors->setMyData(static_cast<int>(MovieScraperInfos::Actors));
    ui->chkBackdrop->setMyData(static_cast<int>(MovieScraperInfos::Backdrop));
    ui->chkCertification->setMyData(static_cast<int>(MovieScraperInfos::Certification));
    ui->chkCountries->setMyData(static_cast<int>(MovieScraperInfos::Countries));
    ui->chkDirector->setMyData(static_cast<int>(MovieScraperInfos::Director));
    ui->chkGenres->setMyData(static_cast<int>(MovieScraperInfos::Genres));
    ui->chkOverview->setMyData(static_cast<int>(MovieScraperInfos::Overview));
    ui->chkPoster->setMyData(static_cast<int>(MovieScraperInfos::Poster));
    ui->chkRating->setMyData(static_cast<int>(MovieScraperInfos::Rating));
    ui->chkReleased->setMyData(static_cast<int>(MovieScraperInfos::Released));
    ui->chkRuntime->setMyData(static_cast<int>(MovieScraperInfos::Runtime));
    ui->chkSet->setMyData(static_cast<int>(MovieScraperInfos::Set));
    ui->chkStudios->setMyData(static_cast<int>(MovieScraperInfos::Studios));
    ui->chkTagline->setMyData(static_cast<int>(MovieScraperInfos::Tagline));
    ui->chkTitle->setMyData(static_cast<int>(MovieScraperInfos::Title));
    ui->chkTrailer->setMyData(static_cast<int>(MovieScraperInfos::Trailer));
    ui->chkWriter->setMyData(static_cast<int>(MovieScraperInfos::Writer));
    ui->chkLogo->setMyData(static_cast<int>(MovieScraperInfos::Logo));
    ui->chkClearArt->setMyData(static_cast<int>(MovieScraperInfos::ClearArt));
    ui->chkCdArt->setMyData(static_cast<int>(MovieScraperInfos::CdArt));
    ui->chkBanner->setMyData(static_cast<int>(MovieScraperInfos::Banner));
    ui->chkThumb->setMyData(static_cast<int>(MovieScraperInfos::Thumb));
    ui->chkTags->setMyData(static_cast<int>(MovieScraperInfos::Tags));

    for (MyCheckBox* box : ui->groupBox->findChildren<MyCheckBox*>()) {
        if (box->myData().toInt() > 0) {
            connect(box, &QAbstractButton::clicked, this, &MovieMultiScrapeDialog::onChkToggled);
        }
    }
    for (const auto* scraper : Manager::instance()->movieScrapers()) {
        ui->comboScraper->addItem(scraper->name(), scraper->identifier());
    }

    connect(ui->chkUnCheckAll, &QAbstractButton::clicked, this, &MovieMultiScrapeDialog::onChkAllToggled);
    connect(ui->btnStartScraping, &QAbstractButton::clicked, this, &MovieMultiScrapeDialog::onStartScraping);
    connect(ui->comboScraper, SIGNAL(currentIndexChanged(int)), this, SLOT(setCheckBoxesEnabled()));
}

MovieMultiScrapeDialog::~MovieMultiScrapeDialog()
{
    delete ui;
}

MovieMultiScrapeDialog* MovieMultiScrapeDialog::instance(QWidget* parent)
{
    static MovieMultiScrapeDialog* m_instance = nullptr;
    if (m_instance == nullptr) {
        m_instance = new MovieMultiScrapeDialog(parent);
    }
    return m_instance;
}

int MovieMultiScrapeDialog::exec()
{
    m_queue.clear();
    ui->movieCounter->setVisible(false);
    ui->comboScraper->setEnabled(true);
    ui->btnCancel->setVisible(true);
    ui->btnClose->setVisible(false);
    ui->btnStartScraping->setVisible(true);
    ui->btnStartScraping->setEnabled(true);
    ui->chkAutoSave->setEnabled(true);
    ui->chkOnlyImdb->setEnabled(true);
    ui->progressAll->setValue(0);
    ui->progressMovie->setValue(0);
    ui->groupBox->setEnabled(true);
    ui->movie->clear();
    m_currentMovie = nullptr;
    m_executed = true;
    setCheckBoxesEnabled();
    adjustSize();

    ui->chkAutoSave->setChecked(Settings::instance()->multiScrapeSaveEach());
    ui->chkOnlyImdb->setChecked(Settings::instance()->multiScrapeOnlyWithId());
    return QDialog::exec();
}

void MovieMultiScrapeDialog::accept()
{
    for (auto* scraper : Manager::instance()->movieScrapers()) {
        disconnect(scraper, &MovieScraperInterface::searchDone, this, &MovieMultiScrapeDialog::onSearchFinished);
    }
    m_executed = false;
    Settings::instance()->setMultiScrapeOnlyWithId(ui->chkOnlyImdb->isChecked());
    Settings::instance()->setMultiScrapeSaveEach(ui->chkAutoSave->isChecked());
    Settings::instance()->saveSettings();
    QDialog::accept();
}

void MovieMultiScrapeDialog::reject()
{
    for (auto* scraper : Manager::instance()->movieScrapers()) {
        disconnect(scraper, &MovieScraperInterface::searchDone, this, &MovieMultiScrapeDialog::onSearchFinished);
    }
    m_executed = false;
    if (m_currentMovie != nullptr) {
        m_queue.clear();
        m_currentMovie->controller()->abortDownloads();
    }
    Settings::instance()->setMultiScrapeOnlyWithId(ui->chkOnlyImdb->isChecked());
    Settings::instance()->setMultiScrapeSaveEach(ui->chkAutoSave->isChecked());
    Settings::instance()->saveSettings();
    QDialog::reject();
}

void MovieMultiScrapeDialog::setMovies(QVector<Movie*> movies)
{
    m_movies = movies;
}

void MovieMultiScrapeDialog::onStartScraping()
{
    for (auto* scraper : Manager::instance()->movieScrapers()) {
        disconnect(scraper, &MovieScraperInterface::searchDone, this, &MovieMultiScrapeDialog::onSearchFinished);
    }

    ui->groupBox->setEnabled(false);
    ui->comboScraper->setEnabled(false);
    ui->btnStartScraping->setEnabled(false);
    ui->chkAutoSave->setEnabled(false);
    ui->chkOnlyImdb->setEnabled(false);

    m_scraperInterface =
        Manager::instance()->scraper(ui->comboScraper->itemData(ui->comboScraper->currentIndex()).toString());
    if (m_scraperInterface == nullptr) {
        return;
    }

    m_isTmdb = m_scraperInterface->identifier() == TMDb::scraperIdentifier;
    m_isImdb = m_scraperInterface->identifier() == IMDB::scraperIdentifier;

    connect(m_scraperInterface,
        &MovieScraperInterface::searchDone,
        this,
        &MovieMultiScrapeDialog::onSearchFinished,
        Qt::UniqueConnection);

    m_queue.append(m_movies.toList());

    ui->movieCounter->setText(QString("0/%1").arg(m_queue.count()));
    ui->movieCounter->setVisible(true);
    ui->progressAll->setMaximum(m_movies.count());
    scrapeNext();
}

void MovieMultiScrapeDialog::onScrapingFinished()
{
    ui->movieCounter->setVisible(false);
    int numberOfMovies = m_movies.count();
    if (ui->chkOnlyImdb->isChecked()) {
        numberOfMovies = 0;
        for (Movie* movie : m_movies) {
            if ((m_isImdb && movie->imdbId().isValid())
                || (m_isTmdb && (movie->imdbId().isValid() || movie->tmdbId().isValid()))) {
                numberOfMovies++;
            }
        }
    }
    ui->movie->setText(tr("Scraping of %n movies has finished.", "", numberOfMovies));
    ui->progressAll->setValue(ui->progressAll->maximum());
    ui->btnCancel->setVisible(false);
    ui->btnClose->setVisible(true);
    ui->btnStartScraping->setVisible(false);
}

void MovieMultiScrapeDialog::scrapeNext()
{
    if (!isExecuted()) {
        return;
    }

    if ((m_currentMovie != nullptr) && ui->chkAutoSave->isChecked()) {
        m_currentMovie->controller()->saveData(Manager::instance()->mediaCenterInterface());
    }

    if (m_queue.isEmpty()) {
        onScrapingFinished();
        return;
    }

    m_currentMovie = m_queue.dequeue();

    ui->movie->setText(m_currentMovie->name().trimmed());
    ui->movieCounter->setText(QString("%1/%2").arg(m_movies.count() - m_queue.count()).arg(m_movies.count()));

    ui->progressAll->setValue(ui->progressAll->maximum() - m_queue.size() - 1);
    ui->progressMovie->setValue(0);

    if (ui->chkOnlyImdb->isChecked()
        && ((!m_currentMovie->imdbId().isValid() && m_isImdb)
            || (!m_currentMovie->tmdbId().isValid() && !m_currentMovie->imdbId().isValid() && m_isTmdb)
            || (!m_currentMovie->imdbId().isValid() && !m_currentMovie->tmdbId().isValid()
                && m_scraperInterface->identifier() == CustomMovieScraper::scraperIdentifier))) {
        scrapeNext();
        return;
    }

    connect(m_currentMovie->controller(),
        &MovieController::sigLoadDone,
        this,
        &MovieMultiScrapeDialog::scrapeNext,
        Qt::UniqueConnection);
    connect(m_currentMovie->controller(),
        &MovieController::sigDownloadProgress,
        this,
        &MovieMultiScrapeDialog::onProgress,
        Qt::UniqueConnection);

    m_currentIds.clear();

    if (m_isImdb && m_currentMovie->imdbId().isValid()) {
        loadMovieData(m_currentMovie, m_currentMovie->imdbId());
    } else if (m_isTmdb && m_currentMovie->tmdbId().isValid()) {
        loadMovieData(m_currentMovie, m_currentMovie->tmdbId());
    } else if (m_isTmdb && m_currentMovie->imdbId().isValid()) {
        loadMovieData(m_currentMovie, m_currentMovie->imdbId());
    } else if (m_scraperInterface->identifier() == CustomMovieScraper::scraperIdentifier) {
        if ((CustomMovieScraper::instance()->titleScraper()->identifier() == IMDB::scraperIdentifier
                || CustomMovieScraper::instance()->titleScraper()->identifier() == TMDb::scraperIdentifier)
            && m_currentMovie->imdbId().isValid()) {
            m_scraperInterface->search(m_currentMovie->imdbId().toString());
        } else if (CustomMovieScraper::instance()->titleScraper()->identifier() == TMDb::scraperIdentifier
                   && m_currentMovie->tmdbId().isValid()) {
            m_scraperInterface->search(m_currentMovie->tmdbId().withPrefix());
        } else {
            m_scraperInterface->search(m_currentMovie->name());
        }
    } else {
        m_scraperInterface->search(m_currentMovie->name());
    }
}

void MovieMultiScrapeDialog::loadMovieData(Movie* movie, ImdbId id)
{
    QMap<MovieScraperInterface*, QString> ids;
    ids.insert(nullptr, id.toString());
    movie->controller()->loadData(ids, m_scraperInterface, m_infosToLoad);
}

void MovieMultiScrapeDialog::loadMovieData(Movie* movie, TmdbId id)
{
    QMap<MovieScraperInterface*, QString> ids;
    ids.insert(nullptr, id.toString());
    movie->controller()->loadData(ids, m_scraperInterface, m_infosToLoad);
}

void MovieMultiScrapeDialog::onSearchFinished(QVector<ScraperSearchResult> results)
{
    if (!isExecuted()) {
        return;
    }
    if (results.isEmpty()) {
        scrapeNext();
        return;
    }

    if (m_scraperInterface->identifier() == CustomMovieScraper::scraperIdentifier) {
        auto scraper = dynamic_cast<MovieScraperInterface*>(QObject::sender());
        m_currentIds.insert(scraper, results.first().id);
        QVector<MovieScraperInterface*> searchScrapers =
            CustomMovieScraper::instance()->scrapersNeedSearch(m_infosToLoad, m_currentIds);
        if (!searchScrapers.isEmpty()) {
            connect(searchScrapers.first(),
                &MovieScraperInterface::searchDone,
                this,
                &MovieMultiScrapeDialog::onSearchFinished,
                Qt::UniqueConnection);
            if ((searchScrapers.first()->identifier() == TMDb::scraperIdentifier
                    || searchScrapers.first()->identifier() == IMDB::scraperIdentifier)
                && m_currentMovie->imdbId().isValid()) {
                searchScrapers.first()->search(m_currentMovie->imdbId().toString());
            } else if (searchScrapers.first()->identifier() == TMDb::scraperIdentifier
                       && m_currentMovie->tmdbId().isValid()) {
                searchScrapers.first()->search(m_currentMovie->tmdbId().toString());
            } else {
                searchScrapers.first()->search(m_currentMovie->name());
            }
            return;
        }
    } else {
        m_currentIds.insert(m_scraperInterface, results.first().id);
    }

    m_currentMovie->controller()->loadData(m_currentIds, m_scraperInterface, m_infosToLoad);
}

void MovieMultiScrapeDialog::onProgress(Movie* movie, int current, int maximum)
{
    Q_UNUSED(movie);

    if (!isExecuted()) {
        return;
    }
    ui->progressMovie->setValue(maximum - current);
    ui->progressMovie->setMaximum(maximum);
}

bool MovieMultiScrapeDialog::isExecuted()
{
    return m_executed;
}

void MovieMultiScrapeDialog::onChkToggled()
{
    m_infosToLoad.clear();
    bool allToggled = true;
    for (MyCheckBox* box : ui->groupBox->findChildren<MyCheckBox*>()) {
        if (box->isChecked() && box->myData().toInt() > 0) {
            m_infosToLoad.append(MovieScraperInfos(box->myData().toInt()));
        }
        if (!box->isChecked() && box->myData().toInt() > 0) {
            allToggled = false;
        }
    }

    ui->chkUnCheckAll->setChecked(allToggled);

    QString scraperId = ui->comboScraper->itemData(ui->comboScraper->currentIndex(), Qt::UserRole).toString();
    Settings::instance()->setScraperInfos(MainWidgets::Movies, scraperId, m_infosToLoad);

    ui->btnStartScraping->setEnabled(!m_infosToLoad.isEmpty());
}

void MovieMultiScrapeDialog::onChkAllToggled()
{
    bool checked = ui->chkUnCheckAll->isChecked();
    for (MyCheckBox* box : ui->groupBox->findChildren<MyCheckBox*>()) {
        if (box->myData().toInt() > 0) {
            box->setChecked(checked);
        }
    }
    onChkToggled();
}

void MovieMultiScrapeDialog::setCheckBoxesEnabled()
{
    QString scraperId = ui->comboScraper->itemData(ui->comboScraper->currentIndex(), Qt::UserRole).toString();
    MovieScraperInterface* scraper = Manager::instance()->scraper(scraperId);
    if (scraper == nullptr) {
        return;
    }

    QVector<MovieScraperInfos> scraperSupports = scraper->scraperSupports();
    QVector<MovieScraperInfos> infos = Settings::instance()->scraperInfos<MovieScraperInfos>(scraperId);

    for (MyCheckBox* box : ui->groupBox->findChildren<MyCheckBox*>()) {
        box->setEnabled(scraperSupports.contains(MovieScraperInfos(box->myData().toInt())));
        box->setChecked((infos.contains(MovieScraperInfos(box->myData().toInt())) || infos.isEmpty())
                        && scraperSupports.contains(MovieScraperInfos(box->myData().toInt())));
    }
    onChkToggled();
}
