#include "MovieSearchWidget.h"
#include "ui_MovieSearchWidget.h"

#include "data/Locale.h"
#include "globals/Manager.h"
#include "scrapers/movie/MovieScraper.h"
#include "scrapers/movie/custom/CustomMovieScraper.h"
#include "scrapers/movie/imdb/ImdbMovie.h"
#include "scrapers/movie/tmdb/TmdbMovie.h"
#include "settings/Settings.h"
#include "ui/movies/MoviePreviewAdapter.h"
#include "utils/Meta.h"

#include "log/Log.h"

MovieSearchWidget::MovieSearchWidget(QWidget* parent) : QWidget(parent), ui(new Ui::MovieSearchWidget)
{
    ui->setupUi(this);
    ui->results->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->searchString->setType(MyLineEdit::TypeLoading);

    // clang-format off
    const auto indexChanged = elchOverload<int>(&QComboBox::currentIndexChanged);
    connect(ui->comboScraper,  indexChanged,                    this, &MovieSearchWidget::onScraperChanged, Qt::QueuedConnection);
    connect(ui->comboLanguage, &LanguageCombo::languageChanged, this, &MovieSearchWidget::onLanguageChanged,Qt::QueuedConnection);

    connect(ui->results,       &QTableWidget::itemDoubleClicked,      this, &MovieSearchWidget::onResultDoubleClicked);
    connect(ui->results,       &QTableWidget::currentItemChanged,     this, &MovieSearchWidget::onSelectedResultChanged);
    connect(ui->searchString,  &MyLineEdit::returnPressed,      this, &MovieSearchWidget::startSearch);
    // clang-format on

    connect(ui->searchString, &QLineEdit::textEdited, this, [this](QString searchString) {
        m_searchString = searchString;
    });

    initializeCheckBoxes();
}

MovieSearchWidget::~MovieSearchWidget()
{
    delete ui;
}

const mediaelch::Locale& MovieSearchWidget::scraperLocale() const
{
    return m_currentLanguage;
}

void MovieSearchWidget::abortAndClearResults()
{
    abortCurrentJobs();
    const bool wasBlocked = ui->results->blockSignals(true);
    ui->results->clearContents();
    ui->results->setRowCount(0);
    ui->results->blockSignals(wasBlocked);
}

void MovieSearchWidget::abortCurrentJobs()
{
    if (m_currentSearchJob != nullptr) {
        m_currentSearchJob->kill();
        m_currentSearchJob = nullptr;
    }
    ui->preview->clearAndAbortPreview();
}

void MovieSearchWidget::openAndSearch(QString searchString, const ImdbId& imdbId, const TmdbId& tmdbId)
{
    setupScraperDropdown();

    m_searchString = searchString.replace(QStringLiteral("."), QStringLiteral(" "));
    m_imdbId = imdbId;
    m_tmdbId = tmdbId;

    ui->comboScraper->setEnabled(true);
    ui->detailsGroupBox->setEnabled(true);
    m_customScrapersLeft.clear();
    m_customScraperIds.clear();

    setSearchText();

    onScraperChanged(ui->comboScraper->currentIndex()); // calls startSearch()
}

void MovieSearchWidget::onScrapeSelectedMovie()
{
    if (ui->results->currentItem() != nullptr) {
        onResultDoubleClicked(ui->results->currentItem());
    }
}

void MovieSearchWidget::startSearch()
{
    using namespace mediaelch::scraper;

    emit sigMovieSelectionChanged(false);

    abortAndClearResults();
    ui->comboScraper->setEnabled(false);
    ui->comboLanguage->setEnabled(false);
    ui->searchString->setLoading(true);

    MovieSearchJob::Config config;
    config.locale = m_currentLanguage;
    config.query = ui->searchString->text().trimmed();
    config.includeAdult = Settings::instance()->showAdultScrapers();

    auto* searchJob = m_currentScraper->search(config);
    connect(searchJob, &MovieSearchJob::searchFinished, this, &MovieSearchWidget::onShowResults);
    m_currentSearchJob = searchJob;
    m_currentSearchJob->start();
}

void MovieSearchWidget::setupScraperDropdown()
{
    ui->comboScraper->blockSignals(true);
    ui->comboScraper->clear();

    // Setup new scraper dropdown
    // TODO(Andre): Add common scraper combo box; use it in MovieMultiScrapeDialog.cpp as well
    const bool noAdultScrapers = !Settings::instance()->showAdultScrapers();
    for (const auto* scraper : Manager::instance()->scrapers().movieScrapers()) {
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

    // We must set m_currentScraper for setSearchText() to have effect.
    // Loading settings is not necessary, yet, at this point.
    const QString scraperId = ui->comboScraper->itemData(index, Qt::UserRole).toString();
    auto* scraper = Manager::instance()->scrapers().movieScraper(scraperId);
    m_currentScraper = scraper;
    MediaElch_Ensures(m_currentScraper != nullptr);

    ui->comboScraper->blockSignals(false);
}

int MovieSearchWidget::storedScraperIndex()
{
    // TODO: Also make it use identifiers, not index
    int index = Settings::instance()->currentMovieScraper();
    return (index > 0 && index < ui->comboScraper->count()) ? index : 0;
}

bool MovieSearchWidget::changeScraperTo(const QString& scraperId)
{
    for (int i = 0, count = ui->comboScraper->count(); i < count; ++i) {
        if (ui->comboScraper->itemData(i, Qt::UserRole).toString() == scraperId) {
            // Changing the scraper will trigger a new search.
            ui->comboScraper->setCurrentIndex(i);
            return true;
        }
    }
    qCCritical(generic) << "[Movie Search] Tried to change scraper to non-existing:" << scraperId;
    MediaElch_Unreachable();
}

void MovieSearchWidget::showError(const QString& message)
{
    ui->lblSuccessMessage->hide();
    ui->lblErrorMessage->setText(message);
    ui->lblErrorMessage->show();

    enableSearch();
}

void MovieSearchWidget::showSuccess(const QString& message)
{
    ui->lblErrorMessage->hide();
    ui->lblSuccessMessage->setText(message);
    ui->lblSuccessMessage->show();
    enableSearch();
}

void MovieSearchWidget::enableSearch()
{
    ui->comboScraper->setEnabled(m_customScraperIds.isEmpty());
    ui->comboLanguage->setEnabled(m_customScraperIds.isEmpty() && m_currentScraper != nullptr
                                  && m_currentScraper->meta().supportedLanguages.size() > 1);
    ui->searchString->setLoading(false);
    ui->searchString->setFocus();
}

void MovieSearchWidget::setupLanguageDropdown()
{
    using namespace mediaelch::scraper;

    if (m_currentScraper == nullptr) {
        ui->comboLanguage->setInvalid();
        qCCritical(generic) << "[MovieSearchWidget] Cannot set language dropdown in movie search widget";
        showError(tr("Internal inconsistency: Cannot set language dropdown in movie search widget!"));
        // In debug mode, be strict!
        MediaElch_Debug_Assert(m_currentScraper != nullptr);
        return;
    }

    const auto& meta = m_currentScraper->meta();
    if (meta.identifier != CustomMovieScraper::ID) {
        mediaelch::ScraperConfiguration* scraperSettings =
            Manager::instance()->scrapers().movieScraperConfig(meta.identifier);
        MediaElch_Assert(scraperSettings != nullptr);
        m_currentLanguage = scraperSettings->language();
        ui->comboLanguage->setupLanguages(meta.supportedLanguages, m_currentLanguage);

    } else {
        ui->comboLanguage->setInvalid();
    }
}

void MovieSearchWidget::onShowResults(mediaelch::scraper::MovieSearchJob* searchJob)
{
    using namespace mediaelch::scraper;
    auto dls = makeDeleteLaterScope(searchJob);

    if (searchJob->wasKilled()) {
        // If it was killed, don't report anything.
        return;

    } else if (searchJob->hasError()) {
        qCDebug(generic) << "[MovieSearch] Got error while searching for movie" << searchJob->scraperError().message;
        showError(searchJob->scraperError().message);
        return;
    }

    qCDebug(generic) << "[Search Results] Count: " << searchJob->results().size();
    showSuccess(tr("Found %n results", "", qsizetype_to_int(searchJob->results().size())));


    for (const MovieSearchJob::Result& result : asConst(searchJob->results())) {
        const QString resultName = result.released.isNull()
                                       ? result.title
                                       : QStringLiteral("%1 (%2)").arg(result.title, result.released.toString("yyyy"));

        auto* item = new QTableWidgetItem(resultName);
        item->setData(Qt::UserRole, result.identifier.str());

        const int row = ui->results->rowCount();
        ui->results->insertRow(row);
        ui->results->setItem(row, 0, item);
    }
    ui->results->setCurrentCell(0, 0);
}

void MovieSearchWidget::onSelectedResultChanged(QTableWidgetItem* current, QTableWidgetItem* previous)
{
    using namespace mediaelch::scraper;

    Q_UNUSED(previous);
    if (current == nullptr) {
        // e.g. if table was cleared
        emit sigMovieSelectionChanged(false);
        return;
    }

    MediaElch_Expects(m_currentScraper != nullptr);
    MovieScraper* scraper = m_currentScraper;
    if (isCustomScrapingInProgress()) {
        scraper = Manager::instance()->scrapers().movieScraper(m_customScrapersLeft.first());
    } else if (m_currentScraper->meta().identifier == CustomMovieScraper::ID) {
        // Keep in sync with onResultDoubleClicked()
        scraper = Manager::instance()->scrapers().customMovieScraper().titleScraper();
    }
    MediaElch_Ensures(scraper != nullptr);

    m_scraperMovieId = current->data(Qt::UserRole).toString();
    ui->preview->load(mediaelch::MoviePreviewAdapter::createFor( //
        scraper,
        mediaelch::scraper::MovieIdentifier(m_scraperMovieId),
        m_currentLanguage));

    emit sigMovieSelectionChanged(true);
}

void MovieSearchWidget::onResultDoubleClicked(QTableWidgetItem* item)
{
    using namespace mediaelch::scraper;

    MovieIdentifier currentIdentifier(item->data(Qt::UserRole).toString());

    // For all scrapers except the CustomMovieScraper, we simply emit
    // a signal.  The CustomMovieScraper may need to search on multiple sites.
    if (!isCustomScrapingInProgress() && m_currentScraper->meta().identifier != CustomMovieScraper::ID) {
        m_scraperMovieId = currentIdentifier.str();
        m_customScraperIds.clear();
        emit sigResultClicked();
        return;
    }

    CustomMovieScraper& custom = Manager::instance()->scrapers().customMovieScraper();

    if (!isCustomScrapingInProgress()) {
        // Current scraper is custom movie scraper (title scraper)

        // Disable boxes to indicate that we're in the "custom movie scraper" process.
        ui->comboScraper->setEnabled(false);
        ui->comboLanguage->setEnabled(false);
        ui->detailsGroupBox->setEnabled(false);

        // TODO: Don't use titleScraper(), because it's possible it's not set. Use scrapersNeedSearch().first().
        MovieScraper* titleScraper = custom.titleScraper();
        m_customScraperIds.insert(titleScraper, currentIdentifier);

        const auto scrapers = custom.scrapersNeedSearch(infosToLoad());

        for (const MovieScraper* scraper : scrapers) {
            if (scraper != titleScraper) {
                m_customScrapersLeft << scraper->meta().identifier;
            }
        }
    } else {
        // some other active scraper is used for "custom movie scraper".
        m_customScrapersLeft.removeOne(m_currentScraper->meta().identifier);
        m_customScraperIds.insert(m_currentScraper, currentIdentifier);
    }

    if (m_customScrapersLeft.isEmpty()) {
        m_currentScraper = &custom;
        emit sigResultClicked();

    } else {
        createCustomScraperListLabel();
        changeScraperTo(m_customScrapersLeft.first());
    }
}

void MovieSearchWidget::updateInfoToLoad()
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
}

void MovieSearchWidget::toggleAllInfo(bool checked)
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

QString MovieSearchWidget::scraperId()
{
    return m_currentScraper->meta().identifier;
}

QString MovieSearchWidget::scraperMovieId()
{
    return m_scraperMovieId;
}

QSet<MovieScraperInfo> MovieSearchWidget::infosToLoad()
{
    return m_infosToLoad;
}

void MovieSearchWidget::setCheckBoxesForCurrentScraper()
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

QHash<mediaelch::scraper::MovieScraper*, mediaelch::scraper::MovieIdentifier> MovieSearchWidget::customScraperIds()
{
    return m_customScraperIds;
}

void MovieSearchWidget::onScraperChanged(int index)
{
    if (index < 0 || index >= Manager::instance()->scrapers().movieScrapers().size()) {
        qCCritical(generic) << "[Movie Search] Selected invalid scraper:" << index;
        showError(tr("Internal inconsistency: Selected an invalid scraper!"));
        return;
    }

    const QString scraperId = ui->comboScraper->itemData(index, Qt::UserRole).toString();
    auto* scraper = Manager::instance()->scrapers().movieScraper(scraperId);
    MediaElch_Assert(scraper != nullptr);

    qCDebug(generic) << "[Movie Search] Changed scraper to:" << scraper->meta().identifier;

    m_currentScraper = scraper;
    MediaElch_Assert(m_currentScraper != nullptr);

    if (!isCustomScrapingInProgress()) {
        setCheckBoxesForCurrentScraper();
        // Save currently used scraper.  If the custom movie scraper is in process, don't save it.
        Settings::instance()->setCurrentMovieScraper(index);
    }

    if (m_currentScraper->meta().identifier == mediaelch::scraper::CustomMovieScraper::ID) {
        onCustomMovieScraperSelected();
    } else if (!isCustomScrapingInProgress()) {
        ui->lblCustomScraperInfo->hide();
    }

    setupLanguageDropdown();

    startSearch();
}

void MovieSearchWidget::onLanguageChanged()
{
    const auto& meta = m_currentScraper->meta();
    m_currentLanguage = ui->comboLanguage->currentLocale();

    // Save immediately.
    mediaelch::ScraperConfiguration* scraperSettings =
        Manager::instance()->scrapers().movieScraperConfig(meta.identifier);
    MediaElch_Assert(scraperSettings != nullptr);
    scraperSettings->setLanguage(m_currentLanguage);

    startSearch();
}

void MovieSearchWidget::onCustomMovieScraperSelected()
{
    using namespace mediaelch::scraper;
    auto* scraper = dynamic_cast<CustomMovieScraper*>(m_currentScraper);
    MediaElch_Assert(scraper != nullptr);

    m_customScraperIds.clear();

    createCustomScraperListLabel();
}

void MovieSearchWidget::createCustomScraperListLabel()
{
    using namespace mediaelch::scraper;

    CustomMovieScraper& custom = Manager::instance()->scrapers().customMovieScraper();

    MovieScraper* titleScraper = custom.titleScraper();
    QString currentScraper = m_customScrapersLeft.isEmpty() //
                                 ? titleScraper->meta().identifier
                                 : m_customScrapersLeft.first();

    QString label = tr("The following scrapers need a search result before MediaElch can load all details:");
    label += QStringLiteral("<br/>");

    const auto formatScraperName = [&currentScraper](const MovieScraper* scraper) -> QString {
        if (scraper->meta().identifier == currentScraper) {
            return QStringLiteral("<b>%1</b>").arg(scraper->meta().name);
        } else {
            return scraper->meta().name;
        }
    };

    // Get all scrapers that need searching
    QStringList scraperNames;
    const auto scrapers = custom.scrapersNeedSearch(infosToLoad());
    for (const MovieScraper* movieScraper : scrapers) {
        QString name = formatScraperName(movieScraper);
        if (movieScraper == titleScraper) {
            // title scraper is the first to be scraped: it is not selected in the
            // dropdown menu, so indicate that it's the first
            scraperNames.push_front(name);
        } else {
            scraperNames << name;
        }
    }
    label += scraperNames.join(QStringLiteral(", "));

    ui->lblCustomScraperInfo->setText(label);
    ui->lblCustomScraperInfo->setTextFormat(Qt::RichText);
    ui->lblCustomScraperInfo->show();
}

void MovieSearchWidget::setSearchText()
{
    MediaElch_Expects(m_currentScraper != nullptr);

    QString searchText = [&]() -> QString {
        if (m_currentScraper->meta().identifier == mediaelch::scraper::ImdbMovie::ID && m_imdbId.isValid()) {
            return m_imdbId.toString();
        }
        if (m_currentScraper->meta().identifier == mediaelch::scraper::TmdbMovie::ID) {
            if (m_tmdbId.isValid()) {
                return m_tmdbId.withPrefix();
            }
            if (m_imdbId.isValid()) {
                return m_imdbId.toString();
            }
        }
        return m_searchString;
    }();

    ui->searchString->setText(searchText.trimmed());
}

void MovieSearchWidget::initializeCheckBoxes()
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

    const auto children = ui->detailsGroupBox->findChildren<MyCheckBox*>();
    for (const MyCheckBox* box : children) {
        MediaElch_Debug_Assert(box->myData().toInt() > 0);
        connect(box, &QAbstractButton::clicked, this, &MovieSearchWidget::updateInfoToLoad);
    }
    connect(ui->chkUnCheckAll, &QAbstractButton::clicked, this, &MovieSearchWidget::toggleAllInfo);
}
