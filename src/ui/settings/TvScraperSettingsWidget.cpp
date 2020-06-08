#include "ui/settings/TvScraperSettingsWidget.h"
#include "ui_TvScraperSettingsWidget.h"

#include "globals/Manager.h"
#include "scrapers/concert/ConcertScraperInterface.h"
#include "scrapers/movie/CustomMovieScraper.h"
#include "scrapers/movie/IMDB.h"
#include "scrapers/movie/MovieScraperInterface.h"
#include "scrapers/tv_show/TheTvDb.h"
#include "scrapers/tv_show/TvScraperInterface.h"
#include "settings/Settings.h"

TvScraperSettingsWidget::TvScraperSettingsWidget(QWidget* parent) : QWidget(parent), ui(new Ui::TvScraperSettingsWidget)
{
    ui->setupUi(this);

    ui->tvScraperTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui->tvScraperTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    ui->tvScraperTable->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
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
    QSet<ShowScraperInfo> tvInfos = {ShowScraperInfo::Title,
        ShowScraperInfo::Rating,
        ShowScraperInfo::FirstAired,
        ShowScraperInfo::Runtime,
        ShowScraperInfo::Director,
        ShowScraperInfo::Writer,
        ShowScraperInfo::Certification,
        ShowScraperInfo::Overview,
        ShowScraperInfo::Genres,
        ShowScraperInfo::Tags,
        ShowScraperInfo::Actors};

    ui->tvScraperTable->clearContents();
    ui->tvScraperTable->setRowCount(0);

    for (const auto info : tvInfos) {
        int row = ui->tvScraperTable->rowCount();
        ui->tvScraperTable->insertRow(row);
        ui->tvScraperTable->setItem(row, 0, new QTableWidgetItem(titleForTvScraperInfo(info)));
        ui->tvScraperTable->setCellWidget(row, 1, comboForTvScraperInfo(info));
    }
}

void TvScraperSettingsWidget::saveSettings()
{
    QMap<ShowScraperInfo, QString> tvScraper;
    for (int row = 0, n = ui->tvScraperTable->rowCount(); row < n; ++row) {
        auto box = dynamic_cast<QComboBox*>(ui->tvScraperTable->cellWidget(row, 1));
        ShowScraperInfo info = ShowScraperInfo(box->itemData(0, Qt::UserRole + 1).toInt());
        QString scraper = box->itemData(box->currentIndex()).toString();
        tvScraper.insert(info, scraper);
    }
    m_settings->setCustomTvScraper(tvScraper);
}

QComboBox* TvScraperSettingsWidget::comboForTvScraperInfo(const ShowScraperInfo info)
{
    QString currentScraper = m_settings->customTvScraper().value(info, "notset");

    auto box = new QComboBox();
    box->addItem("The TV DB", TheTvDb::scraperIdentifier);
    box->setItemData(0, static_cast<int>(info), Qt::UserRole + 1);

    box->addItem("IMDB", IMDB::scraperIdentifier);
    box->setItemData(1, static_cast<int>(info), Qt::UserRole + 1);

    if (currentScraper == IMDB::scraperIdentifier) {
        box->setCurrentIndex(1);
    }

    return box;
}

QString TvScraperSettingsWidget::titleForTvScraperInfo(const ShowScraperInfo info)
{
    switch (info) {
    case ShowScraperInfo::Title: return tr("Title");
    case ShowScraperInfo::Rating: return tr("Rating");
    case ShowScraperInfo::FirstAired: return tr("First Aired");
    case ShowScraperInfo::Runtime: return tr("Runtime");
    case ShowScraperInfo::Director: return tr("Director");
    case ShowScraperInfo::Writer: return tr("Writer");
    case ShowScraperInfo::Certification: return tr("Certification");
    case ShowScraperInfo::Overview: return tr("Plot");
    case ShowScraperInfo::Genres: return tr("Genres");
    case ShowScraperInfo::Tags: return tr("Tags");
    case ShowScraperInfo::Actors: return tr("Actors");
    default: return tr("Unsupported");
    }
}
