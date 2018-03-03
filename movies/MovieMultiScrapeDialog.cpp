#include "MovieMultiScrapeDialog.h"
#include "ui_MovieMultiScrapeDialog.h"

#include "globals/Manager.h"
#include "scrapers/CustomMovieScraper.h"
#include "smallWidgets/MyCheckBox.h"

MovieMultiScrapeDialog::MovieMultiScrapeDialog(QWidget *parent) : QDialog(parent), ui(new Ui::MovieMultiScrapeDialog)
{
    ui->setupUi(this);

#ifdef Q_OS_MAC
    setWindowFlags((windowFlags() & ~Qt::WindowType_Mask) | Qt::Sheet);
#else
    setWindowFlags((windowFlags() & ~Qt::WindowType_Mask) | Qt::Dialog);
#endif

    QFont font = ui->movieCounter->font();
#ifdef Q_OS_WIN32
    font.setPointSize(font.pointSize() - 1);
#else
    font.setPointSize(font.pointSize() - 2);
#endif
    ui->movieCounter->setFont(font);

    m_executed = false;
    m_currentMovie = nullptr;

    ui->chkActors->setMyData(MovieScraperInfos::Actors);
    ui->chkBackdrop->setMyData(MovieScraperInfos::Backdrop);
    ui->chkCertification->setMyData(MovieScraperInfos::Certification);
    ui->chkCountries->setMyData(MovieScraperInfos::Countries);
    ui->chkDirector->setMyData(MovieScraperInfos::Director);
    ui->chkGenres->setMyData(MovieScraperInfos::Genres);
    ui->chkOverview->setMyData(MovieScraperInfos::Overview);
    ui->chkPoster->setMyData(MovieScraperInfos::Poster);
    ui->chkRating->setMyData(MovieScraperInfos::Rating);
    ui->chkReleased->setMyData(MovieScraperInfos::Released);
    ui->chkRuntime->setMyData(MovieScraperInfos::Runtime);
    ui->chkSet->setMyData(MovieScraperInfos::Set);
    ui->chkStudios->setMyData(MovieScraperInfos::Studios);
    ui->chkTagline->setMyData(MovieScraperInfos::Tagline);
    ui->chkTitle->setMyData(MovieScraperInfos::Title);
    ui->chkTrailer->setMyData(MovieScraperInfos::Trailer);
    ui->chkWriter->setMyData(MovieScraperInfos::Writer);
    ui->chkLogo->setMyData(MovieScraperInfos::Logo);
    ui->chkClearArt->setMyData(MovieScraperInfos::ClearArt);
    ui->chkCdArt->setMyData(MovieScraperInfos::CdArt);
    ui->chkBanner->setMyData(MovieScraperInfos::Banner);
    ui->chkThumb->setMyData(MovieScraperInfos::Thumb);
    ui->chkTags->setMyData(MovieScraperInfos::Tags);

    foreach (MyCheckBox *box, ui->groupBox->findChildren<MyCheckBox *>()) {
        if (box->myData().toInt() > 0)
            connect(box, SIGNAL(clicked()), this, SLOT(onChkToggled()));
    }
    foreach (ScraperInterface *scraper, Manager::instance()->scrapers())
        ui->comboScraper->addItem(scraper->name(), scraper->identifier());

    connect(ui->chkUnCheckAll, SIGNAL(clicked()), this, SLOT(onChkAllToggled()));
    connect(ui->btnStartScraping, SIGNAL(clicked()), this, SLOT(onStartScraping()));
    connect(ui->comboScraper, SIGNAL(currentIndexChanged(int)), this, SLOT(setChkBoxesEnabled()));
}

MovieMultiScrapeDialog::~MovieMultiScrapeDialog()
{
    delete ui;
}

MovieMultiScrapeDialog *MovieMultiScrapeDialog::instance(QWidget *parent)
{
    static MovieMultiScrapeDialog *m_instance = nullptr;
    if (m_instance == nullptr)
        m_instance = new MovieMultiScrapeDialog(parent);
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
    setChkBoxesEnabled();
    adjustSize();

    ui->chkAutoSave->setChecked(Settings::instance()->multiScrapeSaveEach());
    ui->chkOnlyImdb->setChecked(Settings::instance()->multiScrapeOnlyWithId());
    return QDialog::exec();
}

void MovieMultiScrapeDialog::accept()
{
    foreach (ScraperInterface *scraper, Manager::instance()->scrapers())
        disconnect(scraper,
            SIGNAL(searchDone(QList<ScraperSearchResult>)),
            this,
            SLOT(onSearchFinished(QList<ScraperSearchResult>)));
    m_executed = false;
    Settings::instance()->setMultiScrapeOnlyWithId(ui->chkOnlyImdb->isChecked());
    Settings::instance()->setMultiScrapeSaveEach(ui->chkAutoSave->isChecked());
    Settings::instance()->saveSettings();
    QDialog::accept();
}

void MovieMultiScrapeDialog::reject()
{
    foreach (ScraperInterface *scraper, Manager::instance()->scrapers())
        disconnect(scraper,
            SIGNAL(searchDone(QList<ScraperSearchResult>)),
            this,
            SLOT(onSearchFinished(QList<ScraperSearchResult>)));
    m_executed = false;
    if (m_currentMovie) {
        m_queue.clear();
        m_currentMovie->controller()->abortDownloads();
    }
    Settings::instance()->setMultiScrapeOnlyWithId(ui->chkOnlyImdb->isChecked());
    Settings::instance()->setMultiScrapeSaveEach(ui->chkAutoSave->isChecked());
    Settings::instance()->saveSettings();
    QDialog::reject();
}

void MovieMultiScrapeDialog::setMovies(QList<Movie *> movies)
{
    m_movies = movies;
}

void MovieMultiScrapeDialog::onStartScraping()
{
    foreach (ScraperInterface *scraper, Manager::instance()->scrapers())
        disconnect(scraper,
            SIGNAL(searchDone(QList<ScraperSearchResult>)),
            this,
            SLOT(onSearchFinished(QList<ScraperSearchResult>)));

    ui->groupBox->setEnabled(false);
    ui->comboScraper->setEnabled(false);
    ui->btnStartScraping->setEnabled(false);
    ui->chkAutoSave->setEnabled(false);
    ui->chkOnlyImdb->setEnabled(false);

    m_scraperInterface =
        Manager::instance()->scraper(ui->comboScraper->itemData(ui->comboScraper->currentIndex()).toString());
    if (!m_scraperInterface)
        return;

    m_isTmdb = m_scraperInterface->identifier() == "tmdb";
    m_isImdb = m_scraperInterface->identifier() == "imdb";

    connect(m_scraperInterface,
        SIGNAL(searchDone(QList<ScraperSearchResult>)),
        this,
        SLOT(onSearchFinished(QList<ScraperSearchResult>)),
        Qt::UniqueConnection);

    m_queue.append(m_movies);

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
        foreach (Movie *movie, m_movies) {
            if ((m_isImdb && !movie->id().isEmpty())
                || (m_isTmdb && (!movie->id().isEmpty() || !movie->tmdbId().isEmpty())))
                numberOfMovies++;
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
    if (!isExecuted())
        return;

    if (m_currentMovie && ui->chkAutoSave->isChecked())
        m_currentMovie->controller()->saveData(Manager::instance()->mediaCenterInterface());

    if (m_queue.isEmpty()) {
        onScrapingFinished();
        return;
    }

    m_currentMovie = m_queue.dequeue();

    ui->movie->setText(m_currentMovie->name());
    ui->movieCounter->setText(QString("%1/%2").arg(m_movies.count() - m_queue.count()).arg(m_movies.count()));

    ui->progressAll->setValue(ui->progressAll->maximum() - m_queue.size() - 1);
    ui->progressMovie->setValue(0);

    if (ui->chkOnlyImdb->isChecked()
        && ((m_currentMovie->id().isEmpty() && m_isImdb)
               || (m_currentMovie->tmdbId().isEmpty() && m_currentMovie->id().isEmpty() && m_isTmdb)
               || (m_currentMovie->id().isEmpty() && m_currentMovie->tmdbId().isEmpty()
                      && m_scraperInterface->identifier() == "custom-movie"))) {
        scrapeNext();
        return;
    }

    connect(m_currentMovie->controller(), SIGNAL(sigLoadDone(Movie *)), this, SLOT(scrapeNext()), Qt::UniqueConnection);
    connect(m_currentMovie->controller(),
        SIGNAL(sigDownloadProgress(Movie *, int, int)),
        this,
        SLOT(onProgress(Movie *, int, int)),
        Qt::UniqueConnection);

    m_currentIds.clear();

    if (m_isImdb && !m_currentMovie->id().isEmpty()) {
        loadMovieData(m_currentMovie, m_currentMovie->id());
    } else if (m_isTmdb && !m_currentMovie->tmdbId().isEmpty()) {
        loadMovieData(m_currentMovie, m_currentMovie->tmdbId());
    } else if (m_isTmdb && !m_currentMovie->id().isEmpty()) {
        loadMovieData(m_currentMovie, m_currentMovie->id());
    } else if (m_scraperInterface->identifier() == "custom-movie") {
        if ((CustomMovieScraper::instance()->titleScraper()->identifier() == "imdb"
                || CustomMovieScraper::instance()->titleScraper()->identifier() == "tmdb")
            && !m_currentMovie->id().isEmpty())
            m_scraperInterface->search(m_currentMovie->id());
        else if (CustomMovieScraper::instance()->titleScraper()->identifier() == "tmdb"
                 && !m_currentMovie->tmdbId().isEmpty())
            m_scraperInterface->search("id" + m_currentMovie->tmdbId());
        else
            m_scraperInterface->search(m_currentMovie->name());
    } else {
        m_scraperInterface->search(m_currentMovie->name());
    }
}

void MovieMultiScrapeDialog::loadMovieData(Movie *movie, QString id)
{
    QMap<ScraperInterface *, QString> ids;
    ids.insert(0, id);
    movie->controller()->loadData(ids, m_scraperInterface, m_infosToLoad);
}

void MovieMultiScrapeDialog::onSearchFinished(QList<ScraperSearchResult> results)
{
    if (!isExecuted())
        return;
    if (results.isEmpty()) {
        scrapeNext();
        return;
    }

    if (m_scraperInterface->identifier() == "custom-movie") {
        auto scraper = static_cast<ScraperInterface *>(QObject::sender());
        m_currentIds.insert(scraper, results.first().id);
        QList<ScraperInterface *> searchScrapers =
            CustomMovieScraper::instance()->scrapersNeedSearch(m_infosToLoad, m_currentIds);
        if (!searchScrapers.isEmpty()) {
            connect(searchScrapers.first(),
                SIGNAL(searchDone(QList<ScraperSearchResult>)),
                this,
                SLOT(onSearchFinished(QList<ScraperSearchResult>)),
                Qt::UniqueConnection);
            if ((searchScrapers.first()->identifier() == "tmdb" || searchScrapers.first()->identifier() == "imdb")
                && !m_currentMovie->id().isEmpty())
                searchScrapers.first()->search(m_currentMovie->id());
            else if (searchScrapers.first()->identifier() == "tmdb" && !m_currentMovie->tmdbId().isEmpty()
                     && !m_currentMovie->tmdbId().startsWith("tt"))
                searchScrapers.first()->search("id" + m_currentMovie->tmdbId());
            else if (searchScrapers.first()->identifier() == "tmdb" && !m_currentMovie->tmdbId().isEmpty())
                searchScrapers.first()->search(m_currentMovie->tmdbId());
            else
                searchScrapers.first()->search(m_currentMovie->name());
            return;
        }
    } else {
        m_currentIds.insert(m_scraperInterface, results.first().id);
    }

    m_currentMovie->controller()->loadData(m_currentIds, m_scraperInterface, m_infosToLoad);
}

void MovieMultiScrapeDialog::onProgress(Movie *movie, int current, int maximum)
{
    Q_UNUSED(movie);

    if (!isExecuted())
        return;
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
    foreach (MyCheckBox *box, ui->groupBox->findChildren<MyCheckBox *>()) {
        if (box->isChecked() && box->myData().toInt() > 0)
            m_infosToLoad.append(box->myData().toInt());
        if (!box->isChecked() && box->myData().toInt() > 0)
            allToggled = false;
    }

    ui->chkUnCheckAll->setChecked(allToggled);

    QString scraperId = ui->comboScraper->itemData(ui->comboScraper->currentIndex(), Qt::UserRole).toString();
    Settings::instance()->setScraperInfos(WidgetMovies, scraperId, m_infosToLoad);

    ui->btnStartScraping->setEnabled(!m_infosToLoad.isEmpty());
}

void MovieMultiScrapeDialog::onChkAllToggled()
{
    bool checked = ui->chkUnCheckAll->isChecked();
    foreach (MyCheckBox *box, ui->groupBox->findChildren<MyCheckBox *>()) {
        if (box->myData().toInt() > 0)
            box->setChecked(checked);
    }
    onChkToggled();
}

void MovieMultiScrapeDialog::setChkBoxesEnabled()
{
    QString scraperId = ui->comboScraper->itemData(ui->comboScraper->currentIndex(), Qt::UserRole).toString();
    ScraperInterface *scraper = Manager::instance()->scraper(scraperId);
    if (!scraper)
        return;

    QList<int> scraperSupports = scraper->scraperSupports();
    QList<int> infos = Settings::instance()->scraperInfos(WidgetMovies, scraperId);

    foreach (MyCheckBox *box, ui->groupBox->findChildren<MyCheckBox *>()) {
        box->setEnabled(scraperSupports.contains(box->myData().toInt()));
        box->setChecked((infos.contains(box->myData().toInt()) || infos.isEmpty())
                        && scraperSupports.contains(box->myData().toInt()));
    }
    onChkToggled();
}
