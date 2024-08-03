#include "ui/settings/ScraperSettingsTable.h"
#include "ui_ScraperSettingsTable.h"

#include "globals/Helper.h"
#include "globals/Manager.h"
#include "log/Log.h"
#include "settings/Settings.h"

#include <QListWidgetItem>
#include <QScrollArea>
#include <QStackedWidget>
#include <QVBoxLayout>

namespace {
static const int ROLE_PAGE_INDEX = Qt::UserRole + 1;
static const int ROLE_IS_ADULT = Qt::UserRole + 2;
} // namespace


using namespace mediaelch;

ScraperSettingsTable::ScraperSettingsTable(QWidget* parent) : QWidget(parent), ui(new Ui::ScraperSettingsTable)
{
    ui->setupUi(this);

    connect(ui->scraperList, &QListWidget::currentItemChanged, this, &ScraperSettingsTable::onSelectedScraperChanges);
    connect(ui->chkEnableAdultScrapers, &QCheckBox::toggled, this, &ScraperSettingsTable::onShowAdultScrapersChanged);
}

ScraperSettingsTable::~ScraperSettingsTable()
{
    delete ui;
}

void ScraperSettingsTable::addScraper(const scraper::SettingsTableData& tableData)
{
    auto* item = new QListWidgetItem;
    item->setText(tableData.scraperName());
    item->setData(ROLE_PAGE_INDEX, ui->scraperList->count());
    item->setData(ROLE_IS_ADULT, tableData.isAdultScraper());
    ui->scraperList->addItem(item);

    auto* scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setSizeAdjustPolicy(QScrollArea::AdjustToContents);

    auto* page = new QWidget(scrollArea);
    scrollArea->setWidget(page);

    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(0, 0, 0, 0);

    auto* info = tableData.createScraperInfoWidget();
    if (info != nullptr) {
        layout->addWidget(info);
    }

    auto* view = tableData.createScraperSettingsWidget();
    if (view != nullptr) {
        auto* lblSettings = new QLabel(tr("Settings"), this);
        layout->addWidget(lblSettings);
        QFont font = lblSettings->font();
        font.setBold(true);
        lblSettings->setFont(font);
        layout->addWidget(view);
    }

    layout->addStretch();

    ui->scraperDetailPages->addWidget(scrollArea);

    // select first item
    if (ui->scraperList->count() == 1) {
        ui->scraperList->item(0)->setSelected(true);
    }
}

void ScraperSettingsTable::addDelimiter(const QString& title, const QString& description)
{
    auto* item = new QListWidgetItem;
    item->setText(title);
    item->setData(ROLE_PAGE_INDEX, ui->scraperList->count());
    QFont itemFont = item->font();
    itemFont.setBold(true);
    item->setFont(itemFont);
    ui->scraperList->addItem(item);

    auto* lblTitle = new QLabel(title, this);
    QFont font = lblTitle->font();
    font.setBold(true);
    lblTitle->setFont(font);

    auto* lblDescription = new QLabel(description, this);

    auto* scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setSizeAdjustPolicy(QScrollArea::AdjustToContents);

    auto* page = new QWidget(scrollArea);
    scrollArea->setWidget(page);

    auto* layout = new QVBoxLayout(page);

    layout->addWidget(lblTitle);
    layout->addWidget(lblDescription);

    layout->addStretch();
    ui->scraperDetailPages->addWidget(scrollArea);
}

void ScraperSettingsTable::clear()
{
    ui->scraperList->clear();
    while (ui->scraperDetailPages->count() > 0) {
        QWidget* widget = ui->scraperDetailPages->widget(ui->scraperDetailPages->count() - 1);
        ui->scraperDetailPages->removeWidget(widget);
    }
}

void ScraperSettingsTable::onSelectedScraperChanges(QListWidgetItem* current, QListWidgetItem* prev)
{
    if (current != prev) {
        bool ok = false;
        int index = current->data(ROLE_PAGE_INDEX).toInt(&ok);
        MediaElch_Debug_Assert(ok);
        ui->scraperDetailPages->setCurrentIndex(index);
    }
}

void ScraperSettingsTable::onShowAdultScrapersChanged(bool enabled)
{
    const elch_ssize_t count = ui->scraperDetailPages->count();
    const elch_ssize_t currentIndex = ui->scraperList->currentRow();
    // there may be no active row!
    const bool isCurrentAdult = currentIndex >= 0 && ui->scraperList->currentItem()->data(ROLE_IS_ADULT).toBool();
    int lastNonAdultScraper = 0;

    for (int i = 0; i < count; ++i) {
        QListWidgetItem* item = ui->scraperList->item(i);
        const bool isAdult = item->data(ROLE_IS_ADULT).toBool();
        const bool showRow = enabled || !isAdult;
        item->setHidden(!showRow);

        if (!isAdult && i <= currentIndex) {
            lastNonAdultScraper = i;
        }
    }

    if (!enabled && isCurrentAdult) {
        // Note: This works only because we know that the first entry is
        // never an adult scraper, it is a heading.
        ui->scraperList->setCurrentRow(lastNonAdultScraper);
    }

    emit sigShowAdultScraper(enabled);
}

bool ScraperSettingsTable::showAdultScraper()
{
    return ui->chkEnableAdultScrapers->isChecked();
}

void ScraperSettingsTable::setShowAdultScraper(bool enabled)
{
    const bool wasChecked = ui->chkEnableAdultScrapers->isChecked();
    ui->chkEnableAdultScrapers->setChecked(enabled);
    if (wasChecked == enabled) {
        // manually trigger the "changed"-event
        onShowAdultScrapersChanged(enabled);
    }
}
