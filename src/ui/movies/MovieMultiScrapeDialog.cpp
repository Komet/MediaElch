#include "MovieMultiScrapeDialog.h"
#include "ui_MovieMultiScrapeDialog.h"

#include "globals/Manager.h"
#include "log/Log.h"
#include "scrapers/movie/custom/CustomMovieScraper.h"
#include "scrapers/movie/imdb/ImdbMovie.h"
#include "scrapers/movie/tmdb/TmdbMovie.h"
#include "settings/Settings.h"
#include "ui/small_widgets/MyCheckBox.h"

MovieMultiScrapeDialog::MovieMultiScrapeDialog(QWidget* parent) : QDialog(parent), ui(new Ui::MovieMultiScrapeDialog)
{
    ui->setupUi(this);
    ui->lblError->hide();

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

    initializeCheckBoxes();

    auto indexChanged = elchOverload<int>(&QComboBox::currentIndexChanged);
    connect(ui->btnStartScraping, &QAbstractButton::clicked, this, &MovieMultiScrapeDialog::onStartScraping);
    connect(ui->comboScraper, indexChanged, this, &MovieMultiScrapeDialog::onScraperChanged);
    connect(ui->comboLanguage, &LanguageCombo::languageChanged, this, &MovieMultiScrapeDialog::onLanguageChanged);
}

MovieMultiScrapeDialog::~MovieMultiScrapeDialog()
{
    delete ui;
}

int MovieMultiScrapeDialog::exec()
{
    m_queue.clear();

    setupScraperDropdown();

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
    ui->detailsGroupBox->setEnabled(true);
    ui->movie->clear();

    m_currentScraper = nullptr;
    m_currentMovie = nullptr;
    m_executed = true;

    ui->chkAutoSave->setChecked(Settings::instance()->multiScrapeSaveEach());
    ui->chkOnlyImdb->setChecked(Settings::instance()->multiScrapeOnlyWithId());

    onScraperChanged(ui->comboScraper->currentIndex()); // calls startSearch()

    return QDialog::exec();
}

void MovieMultiScrapeDialog::accept()
{
    m_currentMovie = nullptr;
    m_executed = false;
    MediaElch_Debug_Assert(m_queue.isEmpty());
    m_queue.clear(); // non-owned Movie*, so clear is fine

    Settings::instance()->setMultiScrapeOnlyWithId(ui->chkOnlyImdb->isChecked());
    Settings::instance()->setMultiScrapeSaveEach(ui->chkAutoSave->isChecked());
    Settings::instance()->saveSettings();

    QDialog::accept();
}

void MovieMultiScrapeDialog::reject()
{
    m_executed = false;
    m_queue.clear(); // non-owned Movie*, so clear is fine
    if (m_currentMovie != nullptr) {
        m_currentMovie->controller()->abortDownloads();
        m_currentMovie = nullptr;
        qCInfo(generic) << "[Movie Multi Scraper] Aborted scraping; aborting downloads for current movie";
    }

    Settings::instance()->setMultiScrapeOnlyWithId(ui->chkOnlyImdb->isChecked());
    Settings::instance()->setMultiScrapeSaveEach(ui->chkAutoSave->isChecked());
    Settings::instance()->saveSettings();

    QDialog::reject();
}

void MovieMultiScrapeDialog::setMovies(QVector<Movie*> movies)
{
    // m_queue is filled in onStartScraping()
    m_movies = std::move(movies);
}

void MovieMultiScrapeDialog::setupLanguageDropdown()
{
    using namespace mediaelch::scraper;

    if (m_currentScraper == nullptr) {
        ui->comboLanguage->setInvalid();
        qCCritical(generic) << "[Movie Multi Scraper] Cannot set language dropdown";
        showError(tr("Internal inconsistency: Cannot set language dropdown in movie search widget!"));
        // In debug mode, be strict!
        MediaElch_Debug_Assert(m_currentScraper != nullptr);
        return;
    }

    const auto& meta = m_currentScraper->meta();
    m_currentLanguage = meta.defaultLocale;
    ui->comboLanguage->setupLanguages(meta.supportedLanguages, m_currentLanguage);
}

void MovieMultiScrapeDialog::setupScraperDropdown()
{
    ui->comboScraper->blockSignals(true);
    ui->comboScraper->clear();

    // Setup new scraper dropdown
    // TODO(Andre): Add common scraper combo box; use it in MovieSearchWidget.cpp as well
    const bool noAdultScrapers = !Settings::instance()->showAdultScrapers();
    const auto& movieScrapers = Manager::instance()->scrapers().movieScrapers();
    for (const auto* scraper : movieScrapers) {
        if (noAdultScrapers && scraper->meta().isAdult) {
            continue;
        }
        if (scraper->meta().isAdult) {
            ui->comboScraper->addItem(
                QIcon(":/img/heart_red_open.png"), scraper->meta().name, scraper->meta().identifier);
        } else {
            ui->comboScraper->addItem(scraper->meta().name, scraper->meta().identifier);
        }
    }

    const int index = storedScraperIndex();
    ui->comboScraper->setCurrentIndex(index);

    ui->comboScraper->blockSignals(false);
}

int MovieMultiScrapeDialog::storedScraperIndex()
{
    // TODO: Also make it use identifiers, not index
    int index = Settings::instance()->currentMovieScraper();
    return (index > 0 && index < ui->comboScraper->count()) ? index : 0;
}

void MovieMultiScrapeDialog::onStartScraping()
{
    using namespace mediaelch::scraper;

    ui->detailsGroupBox->setEnabled(false);
    ui->comboScraper->setEnabled(false);
    ui->btnStartScraping->setEnabled(false);
    ui->chkAutoSave->setEnabled(false);
    ui->chkOnlyImdb->setEnabled(false);

    customScrapersToUse = Manager::instance()->scrapers().customMovieScraper().scrapersNeedSearch(m_infosToLoad);

    m_queue.append(m_movies.toList());

    ui->movieCounter->setText(QStringLiteral("0/%1").arg(m_queue.count()));
    ui->movieCounter->setVisible(true);
    ui->progressAll->setMaximum(qsizetype_to_int(m_movies.count()));

    scrapeNext(); // Queue is filled, start scraping them.
}

void MovieMultiScrapeDialog::onScrapingFinished()
{
    using namespace mediaelch::scraper;
    ui->movieCounter->setVisible(false);

    int numberOfMovies = qsizetype_to_int(m_movies.count());
    if (ui->chkOnlyImdb->isChecked()) {
        numberOfMovies = 0;

        const bool isTmdbScraper = (m_currentScraper->meta().identifier == TmdbMovie::ID);
        const bool isImdbScraper = (m_currentScraper->meta().identifier == ImdbMovie::ID);
        for (Movie* movie : asConst(m_movies)) {
            if ((isImdbScraper && movie->imdbId().isValid())
                || (isTmdbScraper && (movie->imdbId().isValid() || movie->tmdbId().isValid()))) {
                ++numberOfMovies;
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
    if (m_currentMovie != nullptr && ui->chkAutoSave->isChecked()) {
        m_currentMovie->controller()->saveData(Manager::instance()->mediaCenterInterface());
    }
    if (m_queue.isEmpty()) {
        qCInfo(generic) << "[Multi Movie Scraper] Finished scraping of" << m_movies.count() << "movies";
        onScrapingFinished();
        return;
    }
    m_currentMovie = m_queue.dequeue();

    ui->movie->setText(m_currentMovie->name().trimmed());
    ui->movieCounter->setText(QStringLiteral("%1/%2").arg(m_movies.count() - m_queue.count()).arg(m_movies.count()));
    ui->progressAll->setValue(ui->progressAll->maximum() - m_queue.size() - 1);
    ui->progressMovie->setValue(0);

    const bool isTmdbScraper = (m_currentScraper->meta().identifier == TmdbMovie::ID);
    const bool isImdbScraper = (m_currentScraper->meta().identifier == ImdbMovie::ID);

    if (ui->chkOnlyImdb->isChecked()
        && ((isImdbScraper && !m_currentMovie->imdbId().isValid())
            || (isTmdbScraper && !m_currentMovie->tmdbId().isValid() && !m_currentMovie->imdbId().isValid()))) {
        // If we must only load movies with an ID, but there isn't any, ignore the movie.
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

    m_currentIds.clear(); // for CustomMovieScraper

    if (m_currentScraper->meta().identifier == CustomMovieScraper::ID) {
        startNextCustomScraperSearch();

    } else if (isImdbScraper && m_currentMovie->imdbId().isValid()) {
        loadMovieData(m_currentMovie, m_currentMovie->imdbId());

    } else if (isTmdbScraper && m_currentMovie->tmdbId().isValid()) {
        loadMovieData(m_currentMovie, m_currentMovie->tmdbId());

    } else if (isTmdbScraper && m_currentMovie->imdbId().isValid()) {
        loadMovieData(m_currentMovie, m_currentMovie->imdbId());

    } else {
        // non-specific, generic scraper
        MovieSearchJob::Config config;
        config.includeAdult = Settings::instance()->showAdultScrapers();
        config.query = m_currentMovie->name();
        config.locale = m_currentScraper->meta().defaultLocale;
        auto* searchJob = m_currentScraper->search(config);
        connect(searchJob, &MovieSearchJob::searchFinished, this, &MovieMultiScrapeDialog::onSearchFinished);
        searchJob->start();
    }
}

void MovieMultiScrapeDialog::loadMovieData(Movie* movie, const ImdbId& id)
{
    using namespace mediaelch::scraper;
    QHash<MovieScraper*, MovieIdentifier> ids{{m_currentScraper, MovieIdentifier(id)}};
    movie->controller()->loadData(ids, m_currentLanguage, m_infosToLoad);
}

void MovieMultiScrapeDialog::loadMovieData(Movie* movie, const TmdbId& id)
{
    using namespace mediaelch::scraper;
    QHash<MovieScraper*, MovieIdentifier> ids{{m_currentScraper, MovieIdentifier(id)}};
    movie->controller()->loadData(ids, m_currentLanguage, m_infosToLoad);
}

void MovieMultiScrapeDialog::onSearchFinished(mediaelch::scraper::MovieSearchJob* searchJob)
{
    using namespace mediaelch::scraper;
    auto dls = makeDeleteLaterScope(searchJob);

    MediaElch_Debug_Expects(m_currentScraper->meta().identifier != CustomMovieScraper::ID);

    if (!isExecuted()) {
        // do nothing; should only happen (if at all) if a search job finishes after we've closed the window.

    } else if (searchJob->hasError()) {
        showError(searchJob->errorString());
        scrapeNext();

    } else if (searchJob->results().isEmpty()) {
        scrapeNext(); // silently ignore

    } else {
        // Non-custom-movie scraper found a title => scrape it
        MovieIdentifier id = searchJob->results().first().identifier;
        m_currentMovie->controller()->loadData({{m_currentScraper, id}}, m_currentLanguage, m_infosToLoad);
    }
}

void MovieMultiScrapeDialog::onCustomScraperSearchFinished(mediaelch::scraper::MovieSearchJob* searchJob)
{
    using namespace mediaelch::scraper;

    // Custom Movie Scraper: We have to search first using all specified scrapers.
    QString scraperId = searchJob->property("scraper").value<QString>();
    if (scraperId.isEmpty()) {
        qCCritical(generic) << "[MovieMultiScraperDialog] Could not get scraper from search job! Invalid QVariant";
        MediaElch_Debug_Assert(!scraperId.isEmpty());
        scrapeNext();
        return;
    }

    auto* scraper = Manager::instance()->scrapers().movieScraper(scraperId);
    if (scraper == nullptr) {
        qCCritical(generic) << "[MovieMultiScraperDialog] Could not get scraper from search job!";
        MediaElch_Debug_Assert(scraper != nullptr);
        scrapeNext();
        return;
    }

    if (searchJob->hasError() || searchJob->results().isEmpty()) {
        // Placeholder; we still want to search with all other scrapers
        m_currentIds.insert(scraper, MovieIdentifier{""});
    } else {
        m_currentIds.insert(scraper, searchJob->results().first().identifier);
    }

    startNextCustomScraperSearch();
}

void MovieMultiScrapeDialog::startNextCustomScraperSearch()
{
    using namespace mediaelch::scraper;

    QVector<MovieScraper*> scrapersLeft = remainingScrapersForCustomScraperSearch();

    if (scrapersLeft.isEmpty()) {
        // TODO: loadData expects there to be at least 2 entries for the custom movie scraper.
        //       This hack should not be necessary!
        if (m_currentIds.size() == 1) {
            m_currentIds.insert(nullptr, MovieIdentifier(""));
        }
        // We have everything we need => start scraping.
        m_currentMovie->controller()->loadData(m_currentIds, m_currentLanguage, m_infosToLoad);

    } else {
        // TODO: Can we deduplicate this as well?
        MovieScraper* nextScraper = scrapersLeft.first();
        MediaElch_Debug_Assert(nextScraper != nullptr);

        mediaelch::scraper::MovieSearchJob::Config config;
        config.includeAdult = Settings::instance()->showAdultScrapers();
        config.locale = nextScraper->meta().defaultLocale;

        const bool isTmdbScraper = (m_currentScraper->meta().identifier == TmdbMovie::ID);
        const bool isImdbScraper = (m_currentScraper->meta().identifier == ImdbMovie::ID);

        if ((isTmdbScraper || isImdbScraper) && m_currentMovie->imdbId().isValid()) {
            config.query = m_currentMovie->imdbId().toString();

        } else if (isTmdbScraper && m_currentMovie->tmdbId().isValid()) {
            config.query = m_currentMovie->tmdbId().withPrefix();

        } else {
            config.query = m_currentMovie->name().replace('.', ' ');
        }

        MovieSearchJob* nextSearchJob = nextScraper->search(config);
        MediaElch_Expects(nextSearchJob != nullptr);
        nextSearchJob->setProperty("scraper", nextScraper->meta().identifier);
        connect(nextSearchJob,
            &MovieSearchJob::searchFinished,
            this,
            &MovieMultiScrapeDialog::onCustomScraperSearchFinished);
        nextSearchJob->start();
    }
}

QVector<mediaelch::scraper::MovieScraper*> MovieMultiScrapeDialog::remainingScrapersForCustomScraperSearch() const
{
    // TODO: Use some sort of intersection-operator function.
    //       QSet has intersect; something like that.
    QVector<mediaelch::scraper::MovieScraper*> scrapersLeft;
    for (mediaelch::scraper::MovieScraper* searchScraper : asConst(customScrapersToUse)) {
        if (!m_currentIds.contains(searchScraper)) {
            scrapersLeft << searchScraper;
        }
    }
    return scrapersLeft;
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

void MovieMultiScrapeDialog::updateInfoToLoad()
{
    m_infosToLoad.clear();
    int enabledBoxCount = 0;
    const auto checkBoxes = ui->detailsGroupBox->findChildren<MyCheckBox*>();

    // Rebuild list of information to load
    for (const MyCheckBox* box : checkBoxes) {
        if (!box->isEnabled()) {
            continue;
        }
        ++enabledBoxCount;
        const int info = box->myData().toInt();
        if (info > 0 && box->isChecked()) {
            m_infosToLoad.insert(MovieScraperInfo(info));
        }
    }

    bool allToggled = (m_infosToLoad.size() == enabledBoxCount);
    ui->chkUnCheckAll->setChecked(allToggled);

    Settings::instance()->setScraperInfos(m_currentScraper->meta().identifier, m_infosToLoad);

    ui->btnStartScraping->setEnabled(!m_infosToLoad.isEmpty());
}

void MovieMultiScrapeDialog::toggleAllInfo(bool checked)
{
    const auto& boxes = ui->detailsGroupBox->findChildren<MyCheckBox*>();
    for (MyCheckBox* box : boxes) {
        if (box->myData().toInt() > 0 && box->isEnabled()) {
            box->setChecked(checked);
        } else {
            box->setChecked(false);
        }
    }
    updateInfoToLoad();
}

void MovieMultiScrapeDialog::setCheckBoxesForCurrentScraper()
{
    const QSet<MovieScraperInfo>& scraperSupports = m_currentScraper->meta().supportedDetails;
    auto enabledInfos = Settings::instance()->scraperInfos<MovieScraperInfo>(m_currentScraper->meta().identifier);
    if (enabledInfos.isEmpty()) {
        // It's always never wanted by users to not scrape anything; so in case of invalid
        // settings, enable all checkboxes.
        enabledInfos = mediaelch::scraper::allMovieScraperInfos();
    }

    const auto boxes = ui->detailsGroupBox->findChildren<MyCheckBox*>();
    for (auto* box : boxes) {
        const MovieScraperInfo info = MovieScraperInfo(box->myData().toInt());
        const bool supportsInfo = scraperSupports.contains(info);
        const bool infoActive = supportsInfo && (enabledInfos.contains(info) || enabledInfos.isEmpty());
        box->setEnabled(supportsInfo);
        box->setChecked(infoActive);
    }
    updateInfoToLoad();
}

void MovieMultiScrapeDialog::onScraperChanged(int index)
{
    if (index < 0 || index >= Manager::instance()->scrapers().movieScrapers().size()) {
        qCCritical(generic) << "[Multi Movie Search] Selected invalid scraper:" << index;
        showError(tr("Internal inconsistency: Selected an invalid scraper!"));
        return;
    }

    const QString scraperId = ui->comboScraper->itemData(index, Qt::UserRole).toString();
    auto* scraper = Manager::instance()->scrapers().movieScraper(scraperId);
    MediaElch_Assert(scraper != nullptr);

    qCDebug(generic) << "[Multi Movie Search] Changed scraper to:" << scraper->meta().identifier;

    m_currentScraper = scraper;
    MediaElch_Assert(m_currentScraper != nullptr);

    // Save currently used scraper.
    Settings::instance()->setCurrentMovieScraper(index);

    setCheckBoxesForCurrentScraper();
    setupLanguageDropdown();
}

void MovieMultiScrapeDialog::onLanguageChanged()
{
    m_currentLanguage = ui->comboLanguage->currentLocale();
}

void MovieMultiScrapeDialog::showError(const QString& message)
{
    ui->lblError->setText(message);
    ui->lblError->show();
}

void MovieMultiScrapeDialog::initializeCheckBoxes()
{
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

    const auto checkboxes = ui->detailsGroupBox->findChildren<MyCheckBox*>();
    for (MyCheckBox* box : checkboxes) {
        MediaElch_Debug_Assert(box->myData().toInt() > 0);
        connect(box, &QAbstractButton::clicked, this, &MovieMultiScrapeDialog::updateInfoToLoad);
    }
    connect(ui->chkUnCheckAll, &QAbstractButton::clicked, this, &MovieMultiScrapeDialog::toggleAllInfo);
}
