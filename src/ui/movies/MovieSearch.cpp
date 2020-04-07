#include "MovieSearch.h"
#include "ui_MovieSearch.h"

#include "scrapers/movie/MovieScraper.h"

#include <QDebug>

MovieSearch::MovieSearch(QWidget* parent) : QDialog(parent), ui(new Ui::MovieSearch)
{
    ui->setupUi(this);
#ifdef Q_OS_MAC
    setWindowFlags((windowFlags() & ~Qt::WindowType_Mask) | Qt::Sheet);
    setStyleSheet(styleSheet() + " #MovieSearch { border: 1px solid rgba(0, 0, 0, 100); border-top: none; }");
#else
    setWindowFlags((windowFlags() & ~Qt::WindowType_Mask) | Qt::Dialog);
#endif
    connect(ui->buttonClose, &QAbstractButton::clicked, this, &QDialog::reject);
    connect(ui->movieSearchWidget, &MovieSearchWidget::sigResultClicked, this, &QDialog::accept);
}

MovieSearch::~MovieSearch()
{
    delete ui;
}

MovieSearch* MovieSearch::instance(QWidget* parent)
{
    static MovieSearch* m_instance = new MovieSearch(parent);
    return m_instance;
}

int MovieSearch::exec(QString searchString, ImdbId imdbId, TmdbId tmdbId)
{
    qDebug() << "[MovieSearch] Open window";

    QSize newSize;
    newSize.setHeight(parentWidget()->size().height() - 200);
    newSize.setWidth(qMin(600, parentWidget()->size().width() - 400));
    resize(newSize);

    ui->movieSearchWidget->search(searchString, imdbId, tmdbId);
    return QDialog::exec();
}

int MovieSearch::exec()
{
    return 0;
}

QString MovieSearch::scraperId()
{
    return ui->movieSearchWidget->scraperId();
}

QString MovieSearch::scraperMovieId()
{
    return ui->movieSearchWidget->scraperMovieId();
}

QVector<MovieScraperInfos> MovieSearch::infosToLoad()
{
    return ui->movieSearchWidget->infosToLoad();
}

QHash<mediaelch::scraper::MovieScraper*, QString> MovieSearch::customScraperIds()
{
    return ui->movieSearchWidget->customScraperIds();
}
