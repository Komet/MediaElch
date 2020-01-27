#include "MovieSearchWidget.h"
#include "ui_MovieSearchWidget.h"

#include "globals/Manager.h"
#include "scrapers/movie/CustomMovieScraper.h"
#include "scrapers/movie/IMDB.h"
#include "scrapers/movie/MovieScraperInterface.h"
#include "scrapers/movie/TMDb.h"
#include "settings/Settings.h"

#include <QDebug>

MovieSearchWidget::MovieSearchWidget(QWidget* parent) : QWidget(parent), ui(new Ui::MovieSearchWidget)
{
    ui->setupUi(this);
    ui->results->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->searchString->setType(MyLineEdit::TypeLoading);

    // Setup Events
    for (MovieScraperInterface* scraper : Manager::instance()->movieScrapers()) {
        connect(scraper, &MovieScraperInterface::searchDone, this, &MovieSearchWidget::showResults);
    }

    const auto indexChanged = static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged);
    connect(ui->comboScraper, indexChanged, this, &MovieSearchWidget::onScraperChanged, Qt::QueuedConnection);
    connect(ui->comboLanguage, indexChanged, this, &MovieSearchWidget::onLanguageChanged, Qt::QueuedConnection);
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
 * @brief Initialize the MovieSearchWidget and start searching. Called by MovieSearch
 */
void MovieSearchWidget::search(QString searchString, ImdbId id, TmdbId tmdbId)
{
    setupScraperDropdown();
    setupLanguageDropdown();

    m_searchString = searchString.replace(".", " ");
    m_imdbId = id;
    m_tmdbId = tmdbId;

    ui->comboScraper->setEnabled(true);
    ui->comboLanguage->setEnabled(true);
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
    if (m_currentScraper->identifier() == CustomMovieScraper::scraperIdentifier) {
        m_currentCustomScraper = CustomMovieScraper::instance()->titleScraper();
    }
    setCheckBoxesEnabled(m_currentScraper->scraperSupports());
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
    for (const auto* scraper : Manager::instance()->movieScrapers()) {
        if (noAdultScrapers && scraper->isAdult()) {
            continue;
        }
        if (scraper->isAdult()) {
            ui->comboScraper->addItem(QIcon(":/img/heart_red_open.png"), scraper->name(), scraper->identifier());
        } else {
            ui->comboScraper->addItem(scraper->name(), scraper->identifier());
        }
    }

    const int index = currentScraperIndex();
    ui->comboScraper->setCurrentIndex(index);

    const QString scraperId = ui->comboScraper->itemData(index, Qt::UserRole).toString();
    m_currentScraper = Manager::instance()->scraper(scraperId);

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
    ui->comboLanguage->blockSignals(true);
    ui->comboLanguage->clear();
    ui->comboLanguage->show();
    ui->lblLanguage->show();
    if (m_currentScraper == nullptr) {
        ui->comboLanguage->addItem("Error", "error");
        ui->comboLanguage->blockSignals(false);
        qCritical() << "Cannot set language dropdown in movie search widget";
        showError(tr("Internal inconsistency: Cannot set language dropdown in movie search widget!"));
        return;
    }

    const auto supportedLanguages = m_currentScraper->supportedLanguages();
    if (supportedLanguages.size() <= 1) {
        ui->comboLanguage->blockSignals(false);
        ui->comboLanguage->hide();
        ui->lblLanguage->hide();
        return;
    }

    QString defaultLanguageKey = m_currentScraper->defaultLanguageKey();
    m_currentLanguage = defaultLanguageKey;
    m_currentScraper->changeLanguage(defaultLanguageKey); // store the default language

    int i = 0;
    for (const auto& lang : supportedLanguages) {
        ui->comboLanguage->addItem(lang.languageName, lang.languageKey);
        if (lang.languageKey == defaultLanguageKey) {
            ui->comboLanguage->setCurrentIndex(i);
        }
        ++i;
    }

    ui->comboLanguage->blockSignals(false);
}

void MovieSearchWidget::showResults(QVector<ScraperSearchResult> results, ScraperSearchError error)
{
    qDebug() << "[Search Results] Count: " << results.size();

    ui->comboScraper->setEnabled(m_customScraperIds.isEmpty());
    ui->comboLanguage->setEnabled(m_customScraperIds.isEmpty());
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
    if (m_currentScraper->identifier() != CustomMovieScraper::scraperIdentifier && m_customScraperIds.isEmpty()) {
        m_scraperMovieId = item->data(Qt::UserRole).toString();
        m_customScraperIds.clear();
        emit sigResultClicked();
        return;
    }
    // is custom movie scraper
    ui->comboScraper->setEnabled(false);
    ui->comboLanguage->setEnabled(false);
    ui->groupBox->setEnabled(false);

    if (m_currentCustomScraper == CustomMovieScraper::instance()->titleScraper()) {
        m_customScraperIds.clear();
    }

    m_customScraperIds.insert(m_currentCustomScraper, item->data(Qt::UserRole).toString());
    QVector<MovieScraperInterface*> scrapers =
        CustomMovieScraper::instance()->scrapersNeedSearch(infosToLoad(), m_customScraperIds);

    if (scrapers.isEmpty()) {
        m_currentScraper = CustomMovieScraper::instance();
        emit sigResultClicked();

    } else {
        m_currentCustomScraper = scrapers.first();
        for (int i = 0, n = ui->comboScraper->count(); i < n; ++i) {
            if (ui->comboScraper->itemData(i, Qt::UserRole).toString() == m_currentCustomScraper->identifier()) {
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
            m_infosToLoad.append(MovieScraperInfos(info));
        }
    }

    bool allToggled = (m_infosToLoad.size() == enabledBoxCount);
    ui->chkUnCheckAll->setChecked(allToggled);

    Settings::instance()->setScraperInfos(MainWidgets::Movies, m_currentScraper->identifier(), m_infosToLoad);
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
    return m_currentScraper->identifier();
}

QString MovieSearchWidget::scraperMovieId()
{
    return m_scraperMovieId;
}

QVector<MovieScraperInfos> MovieSearchWidget::infosToLoad()
{
    return m_infosToLoad;
}

void MovieSearchWidget::setCheckBoxesEnabled(QVector<MovieScraperInfos> scraperSupports)
{
    const auto enabledInfos = Settings::instance()->scraperInfos<MovieScraperInfos>(m_currentScraper->identifier());

    for (auto box : ui->groupBox->findChildren<MyCheckBox*>()) {
        const MovieScraperInfos info = MovieScraperInfos(box->myData().toInt());
        const bool supportsInfo = scraperSupports.contains(info);
        const bool infoActive = supportsInfo && (enabledInfos.contains(info) || enabledInfos.isEmpty());
        box->setEnabled(supportsInfo);
        box->setChecked(infoActive);
    }
    updateInfoToLoad();
}

QMap<MovieScraperInterface*, QString> MovieSearchWidget::customScraperIds()
{
    return m_customScraperIds;
}

void MovieSearchWidget::onScraperChanged()
{
    int index = ui->comboScraper->currentIndex();
    if (index < 0 || index >= Manager::instance()->movieScrapers().size()) {
        qCritical() << "[Movie Search] Selected invalid scraper:" << index;
        showError(tr("Internal inconsistency: Selected an invalid scraper!"));
        return;
    }

    QString scraperId = ui->comboScraper->itemData(index, Qt::UserRole).toString();
    m_currentScraper = Manager::instance()->scraper(scraperId);

    setupLanguageDropdown();
    startSearch();
}

void MovieSearchWidget::onLanguageChanged()
{
    const int index = ui->comboLanguage->currentIndex();
    const int size = static_cast<int>(m_currentScraper->supportedLanguages().size());
    if (index < 0 || index >= size) {
        return;
    }
    m_currentLanguage = ui->comboLanguage->itemData(index, Qt::UserRole).toString();
    m_currentScraper->changeLanguage(m_currentLanguage);
    startSearch();
}

void MovieSearchWidget::setSearchText(MovieScraperInterface* scraper)
{
    if (scraper == nullptr) {
        return;
    }
    QString searchText = [&]() -> QString {
        if (scraper->identifier() == IMDB::scraperIdentifier && m_imdbId.isValid()) {
            return m_imdbId.toString();
        }
        if (scraper->identifier() == TMDb::scraperIdentifier) {
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

    for (const MyCheckBox* box : ui->groupBox->findChildren<MyCheckBox*>()) {
        if (box->myData().toInt() > 0) {
            connect(box, &QAbstractButton::clicked, this, &MovieSearchWidget::updateInfoToLoad);
        }
    }
    connect(ui->chkUnCheckAll, &QAbstractButton::clicked, this, &MovieSearchWidget::toggleAllInfo);
}
