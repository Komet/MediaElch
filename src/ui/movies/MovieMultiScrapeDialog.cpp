#include "MovieMultiScrapeDialog.h"
#include "ui_MovieMultiScrapeDialog.h"

#include "globals/Manager.h"
#include "scrapers/movie/custom/CustomMovieScraper.h"
#include "scrapers/movie/imdb/ImdbMovie.h"
#include "scrapers/movie/tmdb/TmdbMovie.h"
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

    ui->chkActors->setMyData(static_cast<int>(MovieScraperInfo::Actors));
    ui->chkBackdrop->setMyData(static_cast<int>(MovieScraperInfo::Backdrop));
    ui->chkCertification->setMyData(static_cast<int>(MovieScraperInfo::Certification));
    ui->chkCountries->setMyData(static_cast<int>(MovieScraperInfo::Countries));
    ui->chkDirector->setMyData(static_cast<int>(MovieScraperInfo::Director));
    ui->chkGenres->setMyData(static_cast<int>(MovieScraperInfo::Genres));
    ui->chkOverview->setMyData(static_cast<int>(MovieScraperInfo::Overview));
    ui->chkPoster->setMyData(static_cast<int>(MovieScraperInfo::Poster));
    ui->chkRating->setMyData(static_cast<int>(MovieScraperInfo::Rating));
    ui->chkReleased->setMyData(static_cast<int>(MovieScraperInfo::Released));
    ui->chkRuntime->setMyData(static_cast<int>(MovieScraperInfo::Runtime));
    ui->chkSet->setMyData(static_cast<int>(MovieScraperInfo::Set));
    ui->chkStudios->setMyData(static_cast<int>(MovieScraperInfo::Studios));
    ui->chkTagline->setMyData(static_cast<int>(MovieScraperInfo::Tagline));
    ui->chkTitle->setMyData(static_cast<int>(MovieScraperInfo::Title));
    ui->chkTrailer->setMyData(static_cast<int>(MovieScraperInfo::Trailer));
    ui->chkWriter->setMyData(static_cast<int>(MovieScraperInfo::Writer));
    ui->chkLogo->setMyData(static_cast<int>(MovieScraperInfo::Logo));
    ui->chkClearArt->setMyData(static_cast<int>(MovieScraperInfo::ClearArt));
    ui->chkCdArt->setMyData(static_cast<int>(MovieScraperInfo::CdArt));
    ui->chkBanner->setMyData(static_cast<int>(MovieScraperInfo::Banner));
    ui->chkThumb->setMyData(static_cast<int>(MovieScraperInfo::Thumb));
    ui->chkTags->setMyData(static_cast<int>(MovieScraperInfo::Tags));

    for (MyCheckBox* box : ui->groupBox->findChildren<MyCheckBox*>()) {
        if (box->myData().toInt() > 0) {
            connect(box, &QAbstractButton::clicked, this, &MovieMultiScrapeDialog::onChkToggled);
        }
    }
    for (const auto* scraper : Manager::instance()->scrapers().movieScrapers()) {
        ui->comboScraper->addItem(scraper->meta().name, scraper->meta().identifier);
    }

    connect(ui->chkUnCheckAll, &QAbstractButton::clicked, this, &MovieMultiScrapeDialog::onChkAllToggled);
    connect(ui->btnStartScraping, &QAbstractButton::clicked, this, &MovieMultiScrapeDialog::onStartScraping);
    connect(ui->comboScraper,
        elchOverload<int>(&QComboBox::currentIndexChanged),
        this,
        &MovieMultiScrapeDialog::setCheckBoxesEnabled);
}

MovieMultiScrapeDialog::~MovieMultiScrapeDialog()
{
    delete ui;
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
    setCheckBoxesEnabled(ui->comboScraper->currentIndex());
    adjustSize();

    ui->chkAutoSave->setChecked(Settings::instance()->multiScrapeSaveEach());
    ui->chkOnlyImdb->setChecked(Settings::instance()->multiScrapeOnlyWithId());
    return QDialog::exec();
}

void MovieMultiScrapeDialog::accept()
{
    for (auto* scraper : Manager::instance()->scrapers().movieScrapers()) {
        disconnect(
            scraper, &mediaelch::scraper::MovieScraper::searchDone, this, &MovieMultiScrapeDialog::onSearchFinished);
    }
    m_executed = false;
    Settings::instance()->setMultiScrapeOnlyWithId(ui->chkOnlyImdb->isChecked());
    Settings::instance()->setMultiScrapeSaveEach(ui->chkAutoSave->isChecked());
    Settings::instance()->saveSettings();
    QDialog::accept();
}

void MovieMultiScrapeDialog::reject()
{
    for (auto* scraper : Manager::instance()->scrapers().movieScrapers()) {
        disconnect(
            scraper, &mediaelch::scraper::MovieScraper::searchDone, this, &MovieMultiScrapeDialog::onSearchFinished);
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
    using namespace mediaelch::scraper;

    for (auto* scraper : Manager::instance()->scrapers().movieScrapers()) {
        disconnect(scraper, &MovieScraper::searchDone, this, &MovieMultiScrapeDialog::onSearchFinished);
    }

    ui->groupBox->setEnabled(false);
    ui->comboScraper->setEnabled(false);
    ui->btnStartScraping->setEnabled(false);
    ui->chkAutoSave->setEnabled(false);
    ui->chkOnlyImdb->setEnabled(false);

    m_scraperInterface = Manager::instance()->scrapers().movieScraper(
        ui->comboScraper->itemData(ui->comboScraper->currentIndex()).toString());
    if (m_scraperInterface == nullptr) {
        return;
    }

    m_isTmdb = m_scraperInterface->meta().identifier == TmdbMovie::ID;
    m_isImdb = m_scraperInterface->meta().identifier == ImdbMovie::ID;

    connect(m_scraperInterface,
        &MovieScraper::searchDone,
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
    using namespace mediaelch::scraper;

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
                && m_scraperInterface->meta().identifier == CustomMovieScraper::ID))) {
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
    } else if (m_scraperInterface->meta().identifier == CustomMovieScraper::ID) {
        if ((CustomMovieScraper::instance()->titleScraper()->meta().identifier == ImdbMovie::ID
                || CustomMovieScraper::instance()->titleScraper()->meta().identifier == TmdbMovie::ID)
            && m_currentMovie->imdbId().isValid()) {
            m_scraperInterface->search(m_currentMovie->imdbId().toString());
        } else if (CustomMovieScraper::instance()->titleScraper()->meta().identifier == TmdbMovie::ID
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
    QHash<mediaelch::scraper::MovieScraper*, QString> ids;
    ids.insert(nullptr, id.toString());
    movie->controller()->loadData(ids, m_scraperInterface, m_infosToLoad);
}

void MovieMultiScrapeDialog::loadMovieData(Movie* movie, TmdbId id)
{
    QHash<mediaelch::scraper::MovieScraper*, QString> ids;
    ids.insert(nullptr, id.toString());
    movie->controller()->loadData(ids, m_scraperInterface, m_infosToLoad);
}

void MovieMultiScrapeDialog::onSearchFinished(QVector<ScraperSearchResult> results)
{
    using namespace mediaelch::scraper;

    if (!isExecuted()) {
        return;
    }
    if (results.isEmpty()) {
        scrapeNext();
        return;
    }

    if (m_scraperInterface->meta().identifier == CustomMovieScraper::ID) {
        auto* scraper = dynamic_cast<MovieScraper*>(QObject::sender());
        m_currentIds.insert(scraper, results.first().id);
        QVector<MovieScraper*> searchScrapers =
            CustomMovieScraper::instance()->scrapersNeedSearch(m_infosToLoad, m_currentIds);
        if (!searchScrapers.isEmpty()) {
            connect(searchScrapers.first(),
                &MovieScraper::searchDone,
                this,
                &MovieMultiScrapeDialog::onSearchFinished,
                Qt::UniqueConnection);
            if ((searchScrapers.first()->meta().identifier == TmdbMovie::ID
                    || searchScrapers.first()->meta().identifier == ImdbMovie::ID)
                && m_currentMovie->imdbId().isValid()) {
                searchScrapers.first()->search(m_currentMovie->imdbId().toString());
            } else if (searchScrapers.first()->meta().identifier == TmdbMovie::ID
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

bool MovieMultiScrapeDialog::isExecuted() const
{
    return m_executed;
}

void MovieMultiScrapeDialog::onChkToggled()
{
    m_infosToLoad.clear();
    bool allToggled = true;
    for (MyCheckBox* box : ui->groupBox->findChildren<MyCheckBox*>()) {
        if (box->isChecked() && box->myData().toInt() > 0) {
            m_infosToLoad.insert(MovieScraperInfo(box->myData().toInt()));
        }
        if (!box->isChecked() && box->myData().toInt() > 0) {
            allToggled = false;
        }
    }

    ui->chkUnCheckAll->setChecked(allToggled);

    QString scraperId = ui->comboScraper->itemData(ui->comboScraper->currentIndex(), Qt::UserRole).toString();
    Settings::instance()->setScraperInfos(scraperId, m_infosToLoad);

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

void MovieMultiScrapeDialog::setCheckBoxesEnabled(int index)
{
    QString scraperId = ui->comboScraper->itemData(index, Qt::UserRole).toString();
    mediaelch::scraper::MovieScraper* scraper = Manager::instance()->scrapers().movieScraper(scraperId);
    if (scraper == nullptr) {
        return;
    }

    QSet<MovieScraperInfo> scraperSupports = scraper->meta().supportedDetails;
    QSet<MovieScraperInfo> infos = Settings::instance()->scraperInfos<MovieScraperInfo>(scraperId);

    for (MyCheckBox* box : ui->groupBox->findChildren<MyCheckBox*>()) {
        box->setEnabled(scraperSupports.contains(MovieScraperInfo(box->myData().toInt())));
        box->setChecked((infos.contains(MovieScraperInfo(box->myData().toInt())) || infos.isEmpty())
                        && scraperSupports.contains(MovieScraperInfo(box->myData().toInt())));
    }
    onChkToggled();
}
