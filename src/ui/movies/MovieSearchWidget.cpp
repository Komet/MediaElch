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

    // Setup Events
    for (mediaelch::scraper::MovieScraper* scraper : Manager::instance()->scrapers().movieScrapers()) {
        connect(scraper, &mediaelch::scraper::MovieScraper::searchDone, this, &MovieSearchWidget::showResults);
    }

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
    m_currentScraper->search(ui->searchString->text().trimmed());
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

    ui->comboLanguage->setupLanguages(supportedLocales, mediaelch::Locale(m_currentLanguage));
}

void MovieSearchWidget::showResults(QVector<ScraperSearchResult> results, mediaelch::ScraperError error)
{
    qDebug() << "[Search Results] Count: " << results.size();

    ui->comboScraper->setEnabled(m_customScraperIds.isEmpty());
    ui->comboLanguage->setEnabled(m_customScraperIds.isEmpty() && m_currentScraper != nullptr
                                  && m_currentScraper->meta().supportedLanguages.size() > 1);
    ui->searchString->setLoading(false);
    ui->searchString->setFocus();

    for (const ScraperSearchResult& result : results) {
        const auto resultName = result.released.isNull()
                                    ? result.name
                                    : QString("%1 (%2)").arg(result.name, result.released.toString("yyyy"));

        auto* item = new QTableWidgetItem(resultName);
        item->setData(Qt::UserRole, result.id);

        const int row = ui->results->rowCount();
        ui->results->insertRow(row);
        ui->results->setItem(row, 0, item);
    }

    if (!error.hasError()) {
        showSuccess(tr("Found %n results", "", results.size()));
    } else {
        showError(error.message);
    }
}

void MovieSearchWidget::resultClicked(QTableWidgetItem* item)
{
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

    m_customScraperIds.insert(m_currentCustomScraper, item->data(Qt::UserRole).toString());
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
    for (MyCheckBox* box : ui->groupBox->findChildren<MyCheckBox*>()) {
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

QHash<mediaelch::scraper::MovieScraper*, QString> MovieSearchWidget::customScraperIds()
{
    return m_customScraperIds;
}

void MovieSearchWidget::onScraperChanged()
{
    int index = ui->comboScraper->currentIndex();
    if (index < 0 || index >= Manager::instance()->scrapers().movieScrapers().size()) {
        qCritical() << "[Movie Search] Selected invalid scraper:" << index;
        showError(tr("Internal inconsistency: Selected an invalid scraper!"));
        return;
    }

    QString scraperId = ui->comboScraper->itemData(index, Qt::UserRole).toString();
    m_currentScraper = Manager::instance()->scrapers().movieScraper(scraperId);

    setupLanguageDropdown();
    startSearch();
}

void MovieSearchWidget::onLanguageChanged()
{
    m_currentLanguage = ui->comboLanguage->currentLocale();
    m_currentScraper->changeLanguage(m_currentLanguage);
    startSearch();
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
