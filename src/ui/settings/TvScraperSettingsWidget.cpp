#include "ui/settings/TvScraperSettingsWidget.h"
#include "ui_TvScraperSettingsWidget.h"

#include "globals/Helper.h"
#include "globals/Manager.h"
#include "scrapers/tv_show/TvScraper.h"
#include "settings/ScraperSettings.h"
#include "settings/Settings.h"

#include <QListWidgetItem>

using namespace mediaelch;

TvScraperSettingsWidget::TvScraperSettingsWidget(QWidget* parent) : QWidget(parent), ui(new Ui::TvScraperSettingsWidget)
{
    ui->setupUi(this);

    connect(ui->tvScraperList,
        &QListWidget::currentItemChanged,
        this,
        &TvScraperSettingsWidget::scraperChanged,
        Qt::QueuedConnection);
    connect(ui->comboLanguage,
        &LanguageCombo::languageChanged,
        this,
        &TvScraperSettingsWidget::onLanguageChanged,
        Qt::QueuedConnection);
}

TvScraperSettingsWidget::~TvScraperSettingsWidget()
{
    delete ui;
}

void TvScraperSettingsWidget::setSettings(Settings& settings)
{
    m_settings = &settings;
}

void TvScraperSettingsWidget::loadSettings()
{
    // Cleanup
    ui->tvScraperList->blockSignals(true);
    ui->tvScraperList->clear();
    m_currentScraper = nullptr;

    if (m_settings == nullptr) {
        ui->tvScraperList->blockSignals(false);
        qCritical() << "[TvScraperSettingsWidget] Cannot set up TV scraper widget because settings are undefined.";
        return;
    }

    for (auto* scraper : Manager::instance()->scrapers().tvScrapers()) {
        const auto& id = scraper->meta().identifier;

        auto* item = new QListWidgetItem;
        item->setText(scraper->meta().name);
        item->setData(Qt::UserRole, id);
        ui->tvScraperList->addItem(item);
    }

    m_currentScraper = Manager::instance()->scrapers().tvScrapers().first();
    ui->tvScraperList->item(0)->setSelected(true);
    ui->tvScraperList->blockSignals(false);
    setupScraperDetails();
}

void TvScraperSettingsWidget::saveSettings()
{
    // no-op
    // scraper settings are saved in Settings::saveSettings
}

void TvScraperSettingsWidget::scraperChanged(QListWidgetItem* current, QListWidgetItem* previous)
{
    Q_UNUSED(previous)

    QString scraperId = current->data(Qt::UserRole).toString();
    scraper::TvScraper* scraper = Manager::instance()->scrapers().tvScraper(scraperId);
    if (scraper != nullptr) {
        m_currentScraper = scraper;
        setupScraperDetails();
    } else {
        qCritical() << "[TvScraperSettingsWidget] Could not load scraper for settings!";
    }
}

void TvScraperSettingsWidget::onLanguageChanged()
{
    currentSettings()->setLanguage(ui->comboLanguage->currentLocale());
}

ScraperSettings* TvScraperSettingsWidget::currentSettings()
{
    const auto& id = m_currentScraper->meta().identifier;
    return m_settings->scraperSettings(id);
}

void TvScraperSettingsWidget::setupScraperDetails()
{
    const auto& meta = m_currentScraper->meta();

    ui->txtName->setText(meta.name);
    ui->txtId->setText(meta.identifier);
    ui->txtDescription->setText(meta.description);
    ui->txtWebsite->setText(helper::makeHtmlLink(meta.website));
    ui->txtTOS->setText(helper::makeHtmlLink(meta.termsOfService));
    ui->txtPrivacyPolicy->setText(helper::makeHtmlLink(meta.privacyPolicy));
    ui->txtHelp->setText(helper::makeHtmlLink(meta.help));
    ui->txtInitialized->setText(m_currentScraper->isInitialized() ? tr("Yes") : tr("No"));

    setupLanguageBox();
}

void TvScraperSettingsWidget::setupLanguageBox()
{
    if (m_currentScraper == nullptr) {
        ui->comboLanguage->setInvalid();
        qCritical() << "[TvScraperSettingsWidget] Cannot set language dropdown in TV show search widget";
        return;
    }

    const Locale locale(currentSettings()->language(m_currentScraper->meta().defaultLocale.toString()));
    ui->comboLanguage->setupLanguages(m_currentScraper->meta().supportedLanguages, locale);
}
