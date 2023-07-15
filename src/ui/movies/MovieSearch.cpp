#include "MovieSearch.h"
#include "ui_MovieSearch.h"

#include "scrapers/movie/MovieScraper.h"

#include "log/Log.h"

MovieSearch::MovieSearch(QWidget* parent) : QDialog(parent), ui(new Ui::MovieSearch)
{
    ui->setupUi(this);
#ifdef Q_OS_MAC
    setWindowFlags((windowFlags() & ~Qt::WindowType_Mask) | Qt::Sheet);
    setStyleSheet(styleSheet() + " #MovieSearch { border: 1px solid rgba(0, 0, 0, 100); border-top: none; }");
#else
    setWindowFlags((windowFlags() & ~Qt::WindowType_Mask) | Qt::Dialog);
#endif
    connect(ui->buttonClose, &QAbstractButton::clicked, this, &MovieSearch::reject);
    connect(ui->buttonScrape, &QAbstractButton::clicked, this, &MovieSearch::onScrapeClicked);
    connect(ui->movieSearchWidget, &MovieSearchWidget::sigResultClicked, this, &MovieSearch::accept);
    connect(ui->movieSearchWidget,
        &MovieSearchWidget::sigMovieSelectionChanged,
        this,
        &MovieSearch::onMovieSelectionChanged);
}

MovieSearch::~MovieSearch()
{
    delete ui;
}

/**
 * \brief Executes the search dialog
 * \param searchString Movie name/search string
 * \return Result of QDialog::exec
 * \see MovieSearch::search
 */
int MovieSearch::execWithSearch(QString searchString, ImdbId id, TmdbId tmdbId)
{
    qCDebug(generic) << "[MovieSearch] Open window";

    ui->buttonScrape->setEnabled(false);

    QSize newSize;
    newSize.setHeight(parentWidget()->size().height() - 200);
    newSize.setWidth(qMin(600, parentWidget()->size().width() - 400));
    resize(newSize);

    ui->movieSearchWidget->openAndSearch(searchString, id, tmdbId);
    return exec();
}

void MovieSearch::onScrapeClicked()
{
    ui->movieSearchWidget->onScrapeSelectedMovie();
}

const mediaelch::Locale& MovieSearch::scraperLocale() const
{
    return ui->movieSearchWidget->scraperLocale();
}

QString MovieSearch::scraperId()
{
    return ui->movieSearchWidget->scraperId();
}

/**
 * \brief MovieSearch::scraperId
 * \return Scraper id of the movie last clicked in result table
 */
QString MovieSearch::scraperMovieId()
{
    return ui->movieSearchWidget->scraperMovieId();
}

/**
 * \brief MovieSearch::infosToLoad
 * \return List of infos to load from the scraper
 */
QSet<MovieScraperInfo> MovieSearch::infosToLoad()
{
    return ui->movieSearchWidget->infosToLoad();
}

QHash<mediaelch::scraper::MovieScraper*, mediaelch::scraper::MovieIdentifier> MovieSearch::customScraperIds()
{
    return ui->movieSearchWidget->customScraperIds();
}

void MovieSearch::onMovieSelectionChanged(bool isSelected)
{
    ui->buttonScrape->setEnabled(isSelected);
}
