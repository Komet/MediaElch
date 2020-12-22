#include "ConcertSearchWidget.h"
#include "ui_ConcertSearchWidget.h"

#include "globals/Manager.h"
#include "scrapers/concert/ConcertScraper.h"
#include "scrapers/concert/ConcertSearchJob.h"
#include "settings/Settings.h"
#include "ui/small_widgets/MyCheckBox.h"

using namespace mediaelch::scraper;

ConcertSearchWidget::ConcertSearchWidget(QWidget* parent) : QWidget(parent), ui(new Ui::ConcertSearchWidget)
{
    ui->setupUi(this);

    ui->results->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->searchString->setType(MyLineEdit::TypeLoading);

    // clang-format off
    connect(ui->comboScraper, elchOverload<int>(&QComboBox::currentIndexChanged), this, &ConcertSearchWidget::onScraperChanged, Qt::QueuedConnection);
    connect(ui->comboLanguage, &LanguageCombo::languageChanged, this, &ConcertSearchWidget::onLanguageChanged, Qt::QueuedConnection);
    connect(ui->searchString, &QLineEdit::returnPressed,  this, &ConcertSearchWidget::initializeAndStartSearch);
    connect(ui->results,      &QTableWidget::itemClicked, this, &ConcertSearchWidget::onResultClicked);
    // clang-format on

    ui->chkBackdrop->setMyData(static_cast<int>(ConcertScraperInfo::Backdrop));
    ui->chkCertification->setMyData(static_cast<int>(ConcertScraperInfo::Certification));
    ui->chkExtraArts->setMyData(static_cast<int>(ConcertScraperInfo::ExtraArts));
    ui->chkGenres->setMyData(static_cast<int>(ConcertScraperInfo::Genres));
    ui->chkOverview->setMyData(static_cast<int>(ConcertScraperInfo::Overview));
    ui->chkPoster->setMyData(static_cast<int>(ConcertScraperInfo::Poster));
    ui->chkRating->setMyData(static_cast<int>(ConcertScraperInfo::Rating));
    ui->chkReleased->setMyData(static_cast<int>(ConcertScraperInfo::Released));
    ui->chkRuntime->setMyData(static_cast<int>(ConcertScraperInfo::Runtime));
    ui->chkTagline->setMyData(static_cast<int>(ConcertScraperInfo::Tagline));
    ui->chkTitle->setMyData(static_cast<int>(ConcertScraperInfo::Title));
    ui->chkTrailer->setMyData(static_cast<int>(ConcertScraperInfo::Trailer));

    const auto infoBoxes = ui->concertInfosGroupBox->findChildren<MyCheckBox*>();
    for (MyCheckBox* box : infoBoxes) {
        if (box->myData().toInt() > 0) {
            connect(box, &QAbstractButton::clicked, this, &ConcertSearchWidget::onConcertInfoToggled);
        }
    }

    connect(ui->chkUnCheckAll, &QAbstractButton::clicked, this, &ConcertSearchWidget::onChkAllConcertInfosToggled);

    setupScraperDropdown();
    setupLanguageDropdown();
    updateCheckBoxes();
}

ConcertSearchWidget::~ConcertSearchWidget()
{
    delete ui;
}

void ConcertSearchWidget::clearResultTable()
{
    ui->results->clearContents();
    ui->results->setRowCount(0);
}

void ConcertSearchWidget::search(QString searchString)
{
    ui->searchString->setText(searchString.replace(".", " ").trimmed());
    initializeAndStartSearch();
}

void ConcertSearchWidget::initializeAndStartSearch()
{
    using namespace mediaelch::scraper;

    clearResultTable();

    // Clear messages
    ui->lblErrorMessage->hide();
    ui->lblSuccessMessage->clear();
    ui->lblSuccessMessage->show();

    ui->searchString->setLoading(true);

    if (m_currentScraper->isInitialized()) {
        startSearch();
        return;
    }

    qInfo() << "[ConcertSearch] Scraper is not initialized, wait for initialization:"
            << m_currentScraper->meta().identifier;

    auto* context = new QObject(this);
    connect(m_currentScraper,
        &ConcertScraper::initialized,
        context,
        [this, context](bool wasSuccessful, ConcertScraper* scraper) mutable {
            // if the context is deleted, so is this connection
            context->deleteLater();

            if (scraper != m_currentScraper) {
                return; // scraper has changed in the meantime
            }
            if (wasSuccessful) {
                startSearch();
            } else {
                showError(tr("The %1 scraper could not be initialized!").arg(scraper->meta().name));
            }
        });

    m_currentScraper->initialize();
}

void ConcertSearchWidget::startSearch()
{
    using namespace mediaelch::scraper;

    if (ui->searchString->text().trimmed().isEmpty()) {
        qInfo() << "[ConcertSearch] Search string is empty";
        showError(tr("Please insert a search string!"));
        return;
    }

    qInfo() << "[ConcertSearch] Start search for:" << ui->searchString->text();

    ConcertSearchJob::Config config{ui->searchString->text().trimmed(), m_currentLanguage};
    auto* searchJob = m_currentScraper->search(config);
    connect(searchJob, &ConcertSearchJob::sigFinished, this, &ConcertSearchWidget::onConcertResults);
    searchJob->execute();
}

void ConcertSearchWidget::onConcertResults(ConcertSearchJob* searchJob)
{
    if (searchJob->hasError()) {
        qDebug() << "[ConcertSearch] Got error while searching for concert" << searchJob->error().message;
        showError(searchJob->error().message);
        searchJob->deleteLater();
        return;
    }

    qDebug() << "[ConcertSearch] Result count:" << searchJob->results().count();
    showSuccess(tr("Found %n results", "", searchJob->results().count()));

    for (const auto& result : searchJob->results()) {
        QString title;
        if (result.released.isValid()) {
            title = QStringLiteral("%1 (%2)").arg(result.title, result.released.toString("yyyy"));

        } else {
            title = result.title;
        }

        auto* item = new QTableWidgetItem(title);
        item->setData(Qt::UserRole, result.identifier.str());

        const int row = ui->results->rowCount();
        ui->results->insertRow(row);
        ui->results->setItem(row, 0, item);
    }

    searchJob->deleteLater();
}

void ConcertSearchWidget::onResultClicked(QTableWidgetItem* item)
{
    m_concertIdentifier = item->data(Qt::UserRole).toString();
    emit sigResultClicked();
}

QString ConcertSearchWidget::concertIdentifier() const
{
    return m_concertIdentifier;
}

mediaelch::scraper::ConcertScraper* ConcertSearchWidget::scraper()
{
    return m_currentScraper;
}

const QSet<ConcertScraperInfo>& ConcertSearchWidget::concertDetailsToLoad() const
{
    return m_concertDetailsToLoad;
}

const mediaelch::Locale& ConcertSearchWidget::locale() const
{
    return m_currentLanguage;
}

void ConcertSearchWidget::onConcertInfoToggled()
{
    m_concertDetailsToLoad.clear();
    bool allToggled = true;
    const auto infoBoxes = ui->concertInfosGroupBox->findChildren<MyCheckBox*>();
    for (MyCheckBox* box : infoBoxes) {
        if (box->isEnabled() && box->myData().toInt() > 0) {
            if (box->isChecked()) {
                m_concertDetailsToLoad.insert(ConcertScraperInfo(box->myData().toInt()));
            } else {
                allToggled = false;
            }
        }
    }

    ui->chkUnCheckAll->setChecked(allToggled);

    Settings::instance()->setScraperInfosConcert(m_currentScraper->meta().identifier, m_concertDetailsToLoad);
}

void ConcertSearchWidget::onChkAllConcertInfosToggled()
{
    const bool checked = ui->chkUnCheckAll->isChecked();
    const auto infoBoxes = ui->concertInfosGroupBox->findChildren<MyCheckBox*>();
    for (MyCheckBox* box : infoBoxes) {
        if (box->myData().toInt() > 0 && box->isEnabled()) {
            box->setChecked(checked);
        }
    }
    onConcertInfoToggled();
}


void ConcertSearchWidget::onScraperChanged(int index)
{
    if (index < 0 || index >= Manager::instance()->scrapers().concertScrapers().size()) {
        qCritical() << "[ConcertSearchWidget] Selected invalid scraper:" << index;
        showError(tr("Internal inconsistency: Selected an invalid scraper!"));
        return;
    }

    const QString scraperId = ui->comboScraper->itemData(index, Qt::UserRole).toString();
    qDebug() << "[ConcertSearchWidget] Selected scraper:" << scraperId;
    m_currentScraper = Manager::instance()->scrapers().concertScraper(scraperId);

    if (m_currentScraper == nullptr) {
        qFatal("[ConcertSearchWidget] Couldn't get scraper from manager");
    }

    // Save so that the scraper is auto-selected the next time.
    Settings::instance()->setCurrentConcertScraper(scraperId);

    setupLanguageDropdown();
    updateCheckBoxes();
    initializeAndStartSearch();
}

void ConcertSearchWidget::onLanguageChanged()
{
    const auto& meta = m_currentScraper->meta();
    m_currentLanguage = ui->comboLanguage->currentLocale();

    // Save immediately.
    ScraperSettings* scraperSettings = Settings::instance()->scraperSettings(meta.identifier);
    scraperSettings->setLanguage(m_currentLanguage);
    scraperSettings->save();

    // TODO: Remove this when it is no longer necessary, i.e. when concerts loading is done job based as well.
    m_currentScraper->loadSettings(*scraperSettings);

    initializeAndStartSearch();
}

void ConcertSearchWidget::updateCheckBoxes()
{
    // More fine-grained box-setup below for episode- and show details.
    ui->concertInfosGroupBox->setEnabled(true);

    const auto& meta = m_currentScraper->meta();
    const auto showInfos = Settings::instance()->scraperInfosConcert(meta.identifier);

    const bool showBlocked = ui->concertInfosGroupBox->blockSignals(true);

    const auto infoBoxes = ui->concertInfosGroupBox->findChildren<MyCheckBox*>();
    for (auto* box : infoBoxes) {
        const auto detail = ConcertScraperInfo(box->myData().toInt());
        const bool supported = meta.supportedDetails.contains(detail);
        box->setChecked(showInfos.contains(detail) && supported);
        box->setEnabled(supported);
    }

    ui->concertInfosGroupBox->blockSignals(showBlocked);

    onConcertInfoToggled();
}

void ConcertSearchWidget::setupScraperDropdown()
{
    ui->comboScraper->blockSignals(true);
    ui->comboScraper->clear();

    for (const ConcertScraper* scraper : Manager::instance()->scrapers().concertScrapers()) {
        ui->comboScraper->addItem(scraper->meta().name, scraper->meta().identifier);
    }

    // Get the last selected scraper.
    const QString& currentScraperId = Settings::instance()->currentConcertScraper();
    ConcertScraper* currentScraper = Manager::instance()->scrapers().concertScraper(currentScraperId);

    // The ID may not be a valid scraper. Default to first available scraper.
    if (currentScraper != nullptr) {
        m_currentScraper = currentScraper;
    } else {
        m_currentScraper = Manager::instance()->scrapers().concertScrapers().first();
    }

    const int index = ui->comboScraper->findData(m_currentScraper->meta().identifier);
    ui->comboScraper->setCurrentIndex(index);
    ui->comboScraper->blockSignals(false);
}

void ConcertSearchWidget::setupLanguageDropdown()
{
    const auto& meta = m_currentScraper->meta();
    m_currentLanguage = Settings::instance()->scraperSettings(meta.identifier)->language(meta.defaultLocale);
    ui->comboLanguage->setupLanguages(meta.supportedLanguages, m_currentLanguage);
}

void ConcertSearchWidget::showError(const QString& message)
{
    ui->searchString->setLoading(false);
    ui->searchString->setFocus();
    ui->lblSuccessMessage->hide();
    ui->lblErrorMessage->setText(message);
    ui->lblErrorMessage->show();
}

void ConcertSearchWidget::showSuccess(const QString& message)
{
    ui->searchString->setLoading(false);
    ui->searchString->setFocus();
    ui->lblErrorMessage->hide();
    ui->lblSuccessMessage->setText(message);
    ui->lblSuccessMessage->show();
}
