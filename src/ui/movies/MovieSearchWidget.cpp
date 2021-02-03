#include "MovieSearchWidget.h"
#include "ui_MovieSearchWidget.h"

#include "data/Locale.h"
#include "globals/Manager.h"
#include "globals/Meta.h"
#include "scrapers/movie/MovieScraper.h"
#include "scrapers/movie/custom/CustomMovieScraper.h"
#include "scrapers/movie/imdb/ImdbMovie.h"
#include "scrapers/movie/tmdb/TmdbMovie.h"
#include "settings/Settings.h"

#include <QDebug>

MovieSearchWidget::MovieSearchWidget(QWidget* parent) : QWidget(parent), ui(new Ui::MovieSearchWidget)
{
    ui->setupUi(this);
    ui->results->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->searchString->setType(MyLineEdit::TypeLoading);

    const auto indexChanged = elchOverload<int>(&QComboBox::currentIndexChanged);
    connect(ui->comboScraper, indexChanged, this, &MovieSearchWidget::onScraperChanged, Qt::QueuedConnection);
    connect(ui->comboLanguage,
        &LanguageCombo::languageChanged,
        this,
        &MovieSearchWidget::onLanguageChanged,
        Qt::QueuedConnection);

    connect(ui->results, &QTableWidget::itemClicked, this, &MovieSearchWidget::resultClicked);
    connect(ui->searchString, &MyLineEdit::returnPressed, this, &MovieSearchWidget::startSearch);

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

void MovieSearchWidget::clearResults()
{
    ui->results->clearContents();
    ui->results->setRowCount(0);
}

/**
 * \brief Initialize the MovieSearchWidget and start searching. Called by MovieSearch
 */
void MovieSearchWidget::search(QString searchString, ImdbId id, TmdbId tmdbId)
{
    setupScraperDropdown();
    setupLanguageDropdown();

    m_searchString = searchString.replace(".", " ");
    m_imdbId = id;
    m_tmdbId = tmdbId;

    ui->comboScraper->setEnabled(true);
    ui->groupBox->setEnabled(true);
    m_currentCustomScraper = nullptr;
    m_customScraperIds.clear();

    setSearchText(m_currentScraper);

    startSearch();
}

void MovieSearchWidget::startSearch()
{
    using namespace mediaelch::scraper;

    if (m_currentScraper == nullptr) {
        qWarning() << "Tried to search for movie without active scraper!";
        showError(tr("Cannot scrape a movie without an active scraper!"));
        return;
    }

    if (m_currentScraper->meta().identifier == mediaelch::scraper::CustomMovieScraper::ID) {
        m_currentCustomScraper = mediaelch::scraper::CustomMovieScraper::instance()->titleScraper();
    }

    setCheckBoxesEnabled(m_currentScraper->meta().supportedDetails);
    clearResults();
    ui->comboScraper->setEnabled(false);
    ui->comboLanguage->setEnabled(false);
    ui->searchString->setLoading(true);

    MovieSearchJob::Config config;
    config.locale = m_currentLanguage;
    config.query = ui->searchString->text().trimmed();
    config.includeAdult = Settings::instance()->showAdultScrapers();

    auto* searchJob = m_currentScraper->search(config);
    connect(searchJob, &MovieSearchJob::sigFinished, this, &MovieSearchWidget::showResults);
    searchJob->execute();
    Settings::instance()->setCurrentMovieScraper(ui->comboScraper->currentIndex());
}

void MovieSearchWidget::setupScraperDropdown()
{
    ui->comboScraper->blockSignals(true);
    ui->comboScraper->clear();

    // Setup new scraper dropdown
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

    const int index = currentScraperIndex();
    ui->comboScraper->setCurrentIndex(index);

    const QString scraperId = ui->comboScraper->itemData(index, Qt::UserRole).toString();
    m_currentScraper = Manager::instance()->scrapers().movieScraper(scraperId);

    ui->comboScraper->blockSignals(false);
}

int MovieSearchWidget::currentScraperIndex()
{
    int index = Settings::instance()->currentMovieScraper();
    return (index > 0 && index < ui->comboScraper->count()) ? index : 0;
}

void MovieSearchWidget::showError(const QString& message)
{
    ui->lblSuccessMessage->hide();
    ui->lblErrorMessage->setText(message);
    ui->lblErrorMessage->show();
}

void MovieSearchWidget::showSuccess(const QString& message)
{
    ui->lblErrorMessage->hide();
    ui->lblSuccessMessage->setText(message);
    ui->lblSuccessMessage->show();
}

void MovieSearchWidget::setupLanguageDropdown()
{
    if (m_currentScraper == nullptr) {
        ui->comboLanguage->setInvalid();
        qCritical() << "[MovieSearchWidget] Cannot set language dropdown in movie search widget";
        showError(tr("Internal inconsistency: Cannot set language dropdown in movie search widget!"));
        return;
    }

    const QVector<mediaelch::Locale>& supportedLocales = m_currentScraper->meta().supportedLanguages;
    const mediaelch::Locale defaultLanguage = m_currentScraper->meta().defaultLocale;
    m_currentLanguage = defaultLanguage;
    m_currentScraper->changeLanguage(defaultLanguage); // store the default language
    m_currentScraper->saveSettings(*Settings::instance()->scraperSettings(m_currentScraper->meta().identifier));

    ui->comboLanguage->setupLanguages(supportedLocales, m_currentLanguage);
}

void MovieSearchWidget::showResults(mediaelch::scraper::MovieSearchJob* searchJob)
{
    using namespace mediaelch::scraper;
    auto dls = makeDeleteLaterScope(searchJob);
    if (searchJob->hasError()) {
        showError(searchJob->error().message);
        return;
    }

    qDebug() << "[Search Results] Count: " << searchJob->results().size();
    showSuccess(tr("Found %n results", "", searchJob->results().size()));

    ui->comboScraper->setEnabled(m_customScraperIds.isEmpty());
    ui->comboLanguage->setEnabled(m_customScraperIds.isEmpty() && m_currentScraper != nullptr
                                  && m_currentScraper->meta().supportedLanguages.size() > 1);
    ui->searchString->setLoading(false);
    ui->searchString->setFocus();

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
}

void MovieSearchWidget::resultClicked(QTableWidgetItem* item)
{
    // For all scrapers except the CustomMovieScraper, we simply emit
    // a signal.  The CustomMovieScraper may need to search on multiple sites.
    if (m_currentScraper->meta().identifier != mediaelch::scraper::CustomMovieScraper::ID
        && m_customScraperIds.isEmpty()) {
        m_scraperMovieId = item->data(Qt::UserRole).toString();
        m_customScraperIds.clear();
        emit sigResultClicked();
        return;
    }

    // is custom movie scraper
    ui->comboScraper->setEnabled(false);
    ui->comboLanguage->setEnabled(false);
    ui->groupBox->setEnabled(false);

    if (m_currentCustomScraper == mediaelch::scraper::CustomMovieScraper::instance()->titleScraper()) {
        m_customScraperIds.clear();
    }

    m_customScraperIds.insert(
        m_currentCustomScraper, mediaelch::scraper::MovieIdentifier(item->data(Qt::UserRole).toString()));
    QVector<mediaelch::scraper::MovieScraper*> scrapers =
        mediaelch::scraper::CustomMovieScraper::instance()->scrapersNeedSearch(infosToLoad(), m_customScraperIds);

    if (scrapers.isEmpty()) {
        m_currentScraper = mediaelch::scraper::CustomMovieScraper::instance();
        emit sigResultClicked();

    } else {
        m_currentCustomScraper = scrapers.first();
        for (int i = 0, n = ui->comboScraper->count(); i < n; ++i) {
            if (ui->comboScraper->itemData(i, Qt::UserRole).toString() == m_currentCustomScraper->meta().identifier) {
                ui->comboScraper->setCurrentIndex(i);
                break;
            }
        }
    }
}

void MovieSearchWidget::updateInfoToLoad()
{
    m_infosToLoad.clear();
    int enabledBoxCount = 0;
    const auto checkBoxes = ui->groupBox->findChildren<MyCheckBox*>();

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
    const auto& boxes = ui->groupBox->findChildren<MyCheckBox*>();
    for (MyCheckBox* box : boxes) {
        if (box->myData().toInt() > 0 && box->isEnabled()) {
            box->setChecked(checked);
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

void MovieSearchWidget::setCheckBoxesEnabled(QSet<MovieScraperInfo> scraperSupports)
{
    const auto enabledInfos = Settings::instance()->scraperInfos<MovieScraperInfo>(m_currentScraper->meta().identifier);

    for (auto box : ui->groupBox->findChildren<MyCheckBox*>()) {
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

void MovieSearchWidget::onScraperChanged()
{
    const int index = ui->comboScraper->currentIndex();
    if (index < 0 || index >= Manager::instance()->scrapers().movieScrapers().size()) {
        qCritical() << "[Movie Search] Selected invalid scraper:" << index;
        showError(tr("Internal inconsistency: Selected an invalid scraper!"));
        return;
    }

    const QString scraperId = ui->comboScraper->itemData(index, Qt::UserRole).toString();
    auto* scraper = Manager::instance()->scrapers().movieScraper(scraperId);
    Q_ASSERT(scraper != nullptr);

    m_currentScraper = scraper;

    if (m_currentScraper->meta().identifier == mediaelch::scraper::CustomMovieScraper::ID) {
        onCustomMovieScraperSelected();
    }

    setupLanguageDropdown();
    startSearch();
}

void MovieSearchWidget::onLanguageChanged()
{
    m_currentLanguage = ui->comboLanguage->currentLocale();
    m_currentScraper->changeLanguage(m_currentLanguage);
    startSearch();
}

void MovieSearchWidget::onCustomMovieScraperSelected()
{
    using namespace mediaelch::scraper;
    auto* scraper = dynamic_cast<CustomMovieScraper*>(m_currentScraper);
    Q_ASSERT(scraper != nullptr);

    QString label = tr("The following scrapers need a search result before MediaElch can load all details:");
    label += "<br/>";

    // Get all scrapers that need searching
    auto scrapers = CustomMovieScraper::instance()->scrapersNeedSearch(infosToLoad(), {});
    for (auto* scraper : asConst(scrapers)) {
        label += QStringLiteral("%1<br/>").arg(scraper->meta().name);
    }

    ui->lblCustomScraperInfo->setText(label);
    ui->lblCustomScraperInfo->setTextFormat(Qt::TextFormat::RichText);
}

void MovieSearchWidget::setSearchText(mediaelch::scraper::MovieScraper* scraper)
{
    if (scraper == nullptr) {
        return;
    }
    QString searchText = [&]() -> QString {
        if (scraper->meta().identifier == mediaelch::scraper::ImdbMovie::ID && m_imdbId.isValid()) {
            return m_imdbId.toString();
        }
        if (scraper->meta().identifier == mediaelch::scraper::TmdbMovie::ID) {
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

    for (const MyCheckBox* box : ui->groupBox->findChildren<MyCheckBox*>()) {
        if (box->myData().toInt() > 0) {
            connect(box, &QAbstractButton::clicked, this, &MovieSearchWidget::updateInfoToLoad);
        }
    }
    connect(ui->chkUnCheckAll, &QAbstractButton::clicked, this, &MovieSearchWidget::toggleAllInfo);
}
