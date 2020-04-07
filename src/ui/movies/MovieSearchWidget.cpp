#include "MovieSearchWidget.h"
#include "ui_MovieSearchWidget.h"

#include "globals/Manager.h"
#include "scrapers/movie/IMDB.h"
#include "scrapers/movie/TMDb.h"
#include "settings/Settings.h"

#include <QDebug>

MovieSearchWidget::MovieSearchWidget(QWidget* parent) : QWidget(parent), ui(new Ui::MovieSearchWidget)
{
    ui->setupUi(this);
    ui->results->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->searchString->setType(MyLineEdit::TypeLoading);

    const auto indexChanged = static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged);
    connect(ui->comboScraper, indexChanged, this, &MovieSearchWidget::onScraperChanged, Qt::QueuedConnection);
    connect(ui->comboLanguage, indexChanged, this, &MovieSearchWidget::onLanguageChanged, Qt::QueuedConnection);
    connect(ui->results, &QTableWidget::itemClicked, this, &MovieSearchWidget::resultClicked);
    connect(ui->searchString, &MyLineEdit::returnPressed, this, &MovieSearchWidget::startSearch);
    connect(
        ui->searchString, &MyLineEdit::textEdited, this, [&](QString searchString) { m_searchString = searchString; });

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
        qCritical() << "[MovieSearchWidget] Tried to search for movie without active scraper!";
        showError(tr("Cannot scrape a movie without an active scraper!"));
        return;
    }

    setCheckBoxesEnabled(m_currentScraper->info().scraperSupports);
    clearResults();
    ui->comboScraper->setEnabled(false);
    ui->comboLanguage->setEnabled(false);
    ui->searchString->setLoading(true);

    using namespace mediaelch::scraper;
    MovieSearchJob::Config config(ui->searchString->text().trimmed(), Locale(m_currentLocale));
    auto* searchJob = m_currentScraper->search(config);

    connect(searchJob, &MovieSearchJob::sigSearchSuccess, this, &MovieSearchWidget::onSearchSuccess);
    connect(searchJob, &MovieSearchJob::sigSearchError, this, &MovieSearchWidget::onSearchError);

    Settings::instance()->setCurrentMovieScraper(ui->comboScraper->currentIndex());
}

void MovieSearchWidget::setupScraperDropdown()
{
    ui->comboScraper->blockSignals(true);
    ui->comboScraper->clear();

    // Setup new scraper dropdown
    const bool noAdultScrapers = !Settings::instance()->showAdultScrapers();
    for (const auto* scraper : Manager::instance()->movieScrapers()) {
        if (noAdultScrapers && scraper->info().isAdultScraper) {
            continue;
        }
        if (scraper->info().isAdultScraper) {
            ui->comboScraper->addItem(
                QIcon(":/img/heart_red_open.png"), scraper->info().name, scraper->info().identifier);
        } else {
            ui->comboScraper->addItem(scraper->info().name, scraper->info().identifier);
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

    if (m_currentScraper == nullptr) {
        ui->comboLanguage->addItem("Error", "error");
        ui->comboLanguage->blockSignals(false);
        qCritical() << "[MovieSearchWidget] Cannot set language dropdown in movie search widget";
        showError(tr("Internal inconsistency: Cannot set language dropdown in movie search widget!"));
        return;
    }

    const auto& supportedLanguages = m_currentScraper->info().supportedLanguages;
    if (supportedLanguages.size() <= 1) {
        ui->comboLanguage->hide();
        ui->lblLanguage->hide();
        ui->comboLanguage->blockSignals(false);
        return;
    }

    ui->comboLanguage->show();
    ui->lblLanguage->show();

    QString defaultLocale = m_currentScraper->info().defaultLocale.toString();
    m_currentLocale = defaultLocale;

    int i = 0;
    for (const auto& locale : supportedLanguages) {
        ui->comboLanguage->addItem(locale.languageTranslated(), locale.toString());
        if (locale.toString() == defaultLocale) {
            ui->comboLanguage->setCurrentIndex(i);
        }
        ++i;
    }

    ui->comboLanguage->blockSignals(false);
}

void MovieSearchWidget::onSearchSuccess(QVector<mediaelch::scraper::MovieSearchJob::Result> results)
{
    auto* reply = dynamic_cast<QNetworkReply*>(QObject::sender());
    if (reply != nullptr) {
        reply->deleteLater();
    }

    qInfo() << "[MovieSearchWidget] Search result count:" << results.size();

    ui->comboScraper->setEnabled(m_customScraperIds.isEmpty());
    ui->comboLanguage->setEnabled(m_customScraperIds.isEmpty());
    ui->searchString->setLoading(false);
    ui->searchString->setFocus();

    for (const auto& result : results) {
        const auto resultName = result.released.isNull()
                                    ? result.title
                                    : QString("%1 (%2)").arg(result.title, result.released.toString("yyyy"));

        auto* item = new QTableWidgetItem(resultName);
        item->setData(Qt::UserRole, result.identifier);

        const int row = ui->results->rowCount();
        ui->results->insertRow(row);
        ui->results->setItem(row, 0, item);
    }

    showSuccess(tr("Found %n results", "", results.size()));
}

void MovieSearchWidget::onSearchError(ScraperSearchError error)
{
    auto* reply = dynamic_cast<QNetworkReply*>(QObject::sender());
    if (reply != nullptr) {
        reply->deleteLater();
    }
    showError(error.message);
}

void MovieSearchWidget::resultClicked(QTableWidgetItem* item)
{
    m_scraperMovieId = item->data(Qt::UserRole).toString();
    m_customScraperIds.clear();
    emit sigResultClicked();
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

    Settings::instance()->setScraperInfos(m_currentScraper->info().identifier, m_infosToLoad);
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
    return m_currentScraper->info().identifier;
}

QString MovieSearchWidget::scraperMovieId()
{
    return m_scraperMovieId;
}

QVector<MovieScraperInfos> MovieSearchWidget::infosToLoad()
{
    return m_infosToLoad;
}

void MovieSearchWidget::setCheckBoxesEnabled(const QVector<MovieScraperInfos>& scraperSupports)
{
    const auto enabledInfos =
        Settings::instance()->scraperInfos<MovieScraperInfos>(m_currentScraper->info().identifier);

    for (auto box : ui->groupBox->findChildren<MyCheckBox*>()) {
        const auto info = MovieScraperInfos(box->myData().toInt());
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
    const int size = static_cast<int>(m_currentScraper->info().supportedLanguages.size());
    if (index < 0 || index >= size) {
        return;
    }
    m_currentLocale = ui->comboLanguage->itemData(index, Qt::UserRole).toString();
    startSearch();
}

void MovieSearchWidget::setSearchText(mediaelch::scraper::MovieScraper* scraper)
{
    if (scraper == nullptr) {
        return;
    }
    QString searchText = [&]() -> QString {
        if (scraper->info().identifier == IMDB::scraperIdentifier && m_imdbId.isValid()) {
            return m_imdbId.toString();
        }
        if (scraper->info().identifier == TMDb::scraperIdentifier) {
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
