#include "ui/settings/ScraperSettingsWidget.h"
#include "ui_ScraperSettingsWidget.h"

#include "globals/Manager.h"
#include "scrapers/concert/ConcertScraperInterface.h"
#include "scrapers/movie/CustomMovieScraper.h"
#include "scrapers/movie/IMDB.h"
#include "scrapers/movie/MovieScraperInterface.h"
#include "scrapers/tv_show/TheTvDb.h"
#include "scrapers/tv_show/TvScraperInterface.h"
#include "settings/Settings.h"

ScraperSettingsWidget::ScraperSettingsWidget(QWidget* parent) : QWidget(parent), ui(new Ui::ScraperSettingsWidget)
{
    ui->setupUi(this);

#ifdef Q_OS_MAC
    QFont smallFont = ui->label_18->font();
    smallFont.setPointSize(smallFont.pointSize() - 1);
    ui->label_18->setFont(smallFont);
#endif

    ui->customScraperTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui->customScraperTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    ui->customScraperTable->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    ui->tvScraperTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui->tvScraperTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    ui->tvScraperTable->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    int scraperCounter = 0;
    for (auto* scraper : Manager::instance()->movieScrapers()) {
        if (scraper->hasSettings()) {
            QLabel* name = new QLabel("<b>" + scraper->name() + "</b>");
            name->setAlignment(Qt::AlignRight);
            name->setStyleSheet("margin-top: 3px;");
            ui->gridLayoutScrapers->addWidget(name, scraperCounter, 0);
            ui->gridLayoutScrapers->addWidget(scraper->settingsWidget(), scraperCounter, 1);
            m_scraperRows.insert(scraper, scraperCounter);
            scraperCounter++;
        }
    }
    for (auto* scraper : Manager::instance()->tvScrapers()) {
        if (scraper->hasSettings()) {
            QLabel* name = new QLabel("<b>" + scraper->name() + "</b>");
            name->setAlignment(Qt::AlignRight);
            name->setStyleSheet("margin-top: 3px;");
            ui->gridLayoutScrapers->addWidget(name, scraperCounter, 0);
            ui->gridLayoutScrapers->addWidget(scraper->settingsWidget(), scraperCounter, 1);
            scraperCounter++;
        }
    }
    for (auto* scraper : Manager::instance()->concertScrapers()) {
        if (scraper->hasSettings()) {
            QLabel* name = new QLabel("<b>" + scraper->name() + "</b>");
            name->setAlignment(Qt::AlignRight);
            name->setStyleSheet("margin-top: 3px;");
            ui->gridLayoutScrapers->addWidget(name, scraperCounter, 0);
            ui->gridLayoutScrapers->addWidget(scraper->settingsWidget(), scraperCounter, 1);
            scraperCounter++;
        }
    }
    for (auto* scraper : Manager::instance()->musicScrapers()) {
        if (scraper->hasSettings()) {
            QLabel* name = new QLabel("<b>" + scraper->name() + "</b>");
            name->setAlignment(Qt::AlignRight);
            name->setStyleSheet("margin-top: 3px;");
            ui->gridLayoutScrapers->addWidget(name, scraperCounter, 0);
            ui->gridLayoutScrapers->addWidget(scraper->settingsWidget(), scraperCounter, 1);
            scraperCounter++;
        }
    }

    for (ImageProviderInterface* scraper : Manager::instance()->imageProviders()) {
        if (scraper->hasSettings()) {
            QLabel* name = new QLabel("<b>" + scraper->name() + "</b>");
            name->setAlignment(Qt::AlignRight);
            name->setStyleSheet("margin-top: 3px;");
            ui->gridLayoutScrapers->addWidget(name, scraperCounter, 0);
            ui->gridLayoutScrapers->addWidget(scraper->settingsWidget(), scraperCounter, 1);
            scraperCounter++;
        }
    }

    // clang-format off
    connect(ui->chkEnableAdultScrapers, &QAbstractButton::clicked, this, &ScraperSettingsWidget::onShowAdultScrapers);
    // clang-format on
}

ScraperSettingsWidget::~ScraperSettingsWidget()
{
    delete ui;
}

void ScraperSettingsWidget::setSettings(Settings& settings)
{
    m_settings = &settings;
}

void ScraperSettingsWidget::loadSettings()
{
    ui->chkEnableAdultScrapers->setChecked(m_settings->showAdultScrapers());

    QVector<MovieScraperInfos> infos = {MovieScraperInfos::Title,
        MovieScraperInfos::Set,
        MovieScraperInfos::Tagline,
        MovieScraperInfos::Rating,
        MovieScraperInfos::Released,
        MovieScraperInfos::Runtime,
        MovieScraperInfos::Director,
        MovieScraperInfos::Writer,
        MovieScraperInfos::Certification,
        MovieScraperInfos::Trailer,
        MovieScraperInfos::Overview,
        MovieScraperInfos::Poster,
        MovieScraperInfos::Backdrop,
        MovieScraperInfos::Actors,
        MovieScraperInfos::Genres,
        MovieScraperInfos::Studios,
        MovieScraperInfos::Countries,
        MovieScraperInfos::Logo,
        MovieScraperInfos::ClearArt,
        MovieScraperInfos::CdArt,
        MovieScraperInfos::Banner,
        MovieScraperInfos::Thumb};

    ui->customScraperTable->clearContents();
    ui->customScraperTable->setRowCount(0);

    for (const auto info : infos) {
        int row = ui->customScraperTable->rowCount();
        ui->customScraperTable->insertRow(row);
        ui->customScraperTable->setItem(row, 0, new QTableWidgetItem(titleForMovieScraperInfo(info)));
        ui->customScraperTable->setCellWidget(row, 1, comboForMovieScraperInfo(info));
    }

    QVector<TvShowScraperInfos> tvInfos = {TvShowScraperInfos::Title,
        TvShowScraperInfos::Rating,
        TvShowScraperInfos::FirstAired,
        TvShowScraperInfos::Runtime,
        TvShowScraperInfos::Director,
        TvShowScraperInfos::Writer,
        TvShowScraperInfos::Certification,
        TvShowScraperInfos::Overview,
        TvShowScraperInfos::Genres,
        TvShowScraperInfos::Tags,
        TvShowScraperInfos::Actors};

    ui->tvScraperTable->clearContents();
    ui->tvScraperTable->setRowCount(0);

    for (const auto info : tvInfos) {
        int row = ui->tvScraperTable->rowCount();
        ui->tvScraperTable->insertRow(row);
        ui->tvScraperTable->setItem(row, 0, new QTableWidgetItem(titleForTvScraperInfo(info)));
        ui->tvScraperTable->setCellWidget(row, 1, comboForTvScraperInfo(info));
    }

    onShowAdultScrapers();
}

void ScraperSettingsWidget::saveSettings()
{
    m_settings->setShowAdultScrapers(ui->chkEnableAdultScrapers->isChecked());

    // Custom movie scraper
    QMap<MovieScraperInfos, QString> customMovieScraper;
    for (int row = 0, n = ui->customScraperTable->rowCount(); row < n; ++row) {
        auto box = dynamic_cast<QComboBox*>(ui->customScraperTable->cellWidget(row, 1));
        MovieScraperInfos info = MovieScraperInfos(box->itemData(0, Qt::UserRole + 1).toInt());
        QString scraper = box->itemData(box->currentIndex()).toString();
        customMovieScraper.insert(info, scraper);
    }
    m_settings->setCustomMovieScraper(customMovieScraper);

    // tv scraper
    QMap<TvShowScraperInfos, QString> tvScraper;
    for (int row = 0, n = ui->tvScraperTable->rowCount(); row < n; ++row) {
        auto box = dynamic_cast<QComboBox*>(ui->tvScraperTable->cellWidget(row, 1));
        TvShowScraperInfos info = TvShowScraperInfos(box->itemData(0, Qt::UserRole + 1).toInt());
        QString scraper = box->itemData(box->currentIndex()).toString();
        tvScraper.insert(info, scraper);
    }
    m_settings->setCustomTvScraper(tvScraper);
}

void ScraperSettingsWidget::onShowAdultScrapers()
{
    bool show = ui->chkEnableAdultScrapers->isChecked();
    for (const auto* scraper : Manager::instance()->movieScrapers()) {
        if (scraper->isAdult() && scraper->hasSettings()) {
            ui->gridLayoutScrapers->itemAtPosition(m_scraperRows.value(scraper), 0)->widget()->setVisible(show);
            ui->gridLayoutScrapers->itemAtPosition(m_scraperRows.value(scraper), 1)->widget()->setVisible(show);
        }
    }
}

QComboBox* ScraperSettingsWidget::comboForMovieScraperInfo(const MovieScraperInfos info)
{
    QString currentScraper = m_settings->customMovieScraper().value(info, "notset");

    auto box = new QComboBox();
    int index = 0;
    if (info != MovieScraperInfos::Title) {
        box->addItem(tr("Don't use"), "");
        box->setItemData(0, static_cast<int>(info), Qt::UserRole + 1);
        index = 1;
    }
    for (auto* scraper : Manager::instance()->movieScrapers()) {
        if (scraper->identifier() == CustomMovieScraper::scraperIdentifier) {
            continue;
        }
        if (scraper->scraperNativelySupports().contains(info)) {
            box->addItem(scraper->name(), scraper->identifier());
            box->setItemData(index, static_cast<int>(info), Qt::UserRole + 1);
            if (scraper->identifier() == currentScraper || (currentScraper == "notset" && index == 1)) {
                box->setCurrentIndex(index);
            }
            index++;
        }
    }

    QVector<MovieScraperInfos> images{MovieScraperInfos::Backdrop,
        MovieScraperInfos::Logo,
        MovieScraperInfos::ClearArt,
        MovieScraperInfos::CdArt,
        MovieScraperInfos::Banner,
        MovieScraperInfos::Thumb,
        MovieScraperInfos::Poster};

    if (images.contains(info)) {
        for (const auto img : Manager::instance()->imageProviders()) {
            if (img->identifier() == "images.fanarttv") {
                box->addItem(img->name(), img->identifier());
                box->setItemData(index, static_cast<int>(info), Qt::UserRole + 1);
                if (img->identifier() == currentScraper || (currentScraper == "notset" && index == 1)) {
                    box->setCurrentIndex(index);
                }
                index++;
                break;
            }
        }
    }

    return box;
}

QString ScraperSettingsWidget::titleForMovieScraperInfo(MovieScraperInfos info)
{
    switch (info) {
    case MovieScraperInfos::Title: return tr("Title");
    case MovieScraperInfos::Tagline: return tr("Tagline");
    case MovieScraperInfos::Rating: return tr("Rating");
    case MovieScraperInfos::Released: return tr("Released");
    case MovieScraperInfos::Runtime: return tr("Runtime");
    case MovieScraperInfos::Certification: return tr("Certification");
    case MovieScraperInfos::Trailer: return tr("Trailer");
    case MovieScraperInfos::Overview: return tr("Plot");
    case MovieScraperInfos::Poster: return tr("Poster");
    case MovieScraperInfos::Backdrop: return tr("Fanart");
    case MovieScraperInfos::Actors: return tr("Actors");
    case MovieScraperInfos::Genres: return tr("Genres");
    case MovieScraperInfos::Studios: return tr("Studios");
    case MovieScraperInfos::Countries: return tr("Countries");
    case MovieScraperInfos::Writer: return tr("Writer");
    case MovieScraperInfos::Director: return tr("Director");
    case MovieScraperInfos::Tags: return tr("Tags");
    case MovieScraperInfos::Set: return tr("Set");
    case MovieScraperInfos::Logo: return tr("Logo");
    case MovieScraperInfos::CdArt: return tr("Disc Art");
    case MovieScraperInfos::ClearArt: return tr("Clear Art");
    case MovieScraperInfos::Banner: return tr("Banner");
    case MovieScraperInfos::Thumb: return tr("Thumb");
    default: return tr("Unsupported");
    }
}

QComboBox* ScraperSettingsWidget::comboForTvScraperInfo(const TvShowScraperInfos info)
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

QString ScraperSettingsWidget::titleForTvScraperInfo(const TvShowScraperInfos info)
{
    switch (info) {
    case TvShowScraperInfos::Title: return tr("Title");
    case TvShowScraperInfos::Rating: return tr("Rating");
    case TvShowScraperInfos::FirstAired: return tr("First Aired");
    case TvShowScraperInfos::Runtime: return tr("Runtime");
    case TvShowScraperInfos::Director: return tr("Director");
    case TvShowScraperInfos::Writer: return tr("Writer");
    case TvShowScraperInfos::Certification: return tr("Certification");
    case TvShowScraperInfos::Overview: return tr("Plot");
    case TvShowScraperInfos::Genres: return tr("Genres");
    case TvShowScraperInfos::Tags: return tr("Tags");
    case TvShowScraperInfos::Actors: return tr("Actors");
    default: return tr("Unsupported");
    }
}
