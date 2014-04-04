#include "MovieSearch.h"
#include "ui_MovieSearch.h"
#include "settings/Settings.h"
#include <QDebug>

/**
 * @brief MovieSearch::MovieSearch
 * @param parent
 */
MovieSearch::MovieSearch(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MovieSearch)
{
    ui->setupUi(this);
#ifdef Q_OS_MAC
    setWindowFlags((windowFlags() & ~Qt::WindowType_Mask) | Qt::Sheet);
    setStyleSheet(styleSheet() + " #MovieSearch { border: 1px solid rgba(0, 0, 0, 100); border-top: none; }");
#else
    setWindowFlags((windowFlags() & ~Qt::WindowType_Mask) | Qt::Dialog);
#endif
    connect(ui->buttonClose, SIGNAL(clicked()), this, SLOT(reject()));
    connect(ui->movieSearchWidget, SIGNAL(sigResultClicked()), this, SLOT(accept()));
}

/**
 * @brief MovieSearch::~MovieSearch
 */
MovieSearch::~MovieSearch()
{
    qDebug() << "Trace.";

    delete ui;
}

/**
 * @brief Executes the search dialog
 * @param searchString Movie name/search string
 * @return Result of QDialog::exec
 * @see MovieSearch::search
 */
int MovieSearch::exec(QString searchString, QString id, QString tmdbId)
{
    qDebug() << "Entered";
    QSize newSize;
    newSize.setHeight(parentWidget()->size().height()-200);
    newSize.setWidth(qMin(600, parentWidget()->size().width()-400));
    resize(newSize);

    ui->movieSearchWidget->search(searchString, id, tmdbId);
    return QDialog::exec();
}

int MovieSearch::exec()
{
    return 0;
}

void MovieSearch::accept()
{
    qDebug() << "Trace.";

    Settings::instance()->saveSettings();
    QDialog::accept();
}

void MovieSearch::reject()
{
    qDebug() << "Trace.";

    Settings::instance()->saveSettings();
    QDialog::reject();
}

/*** GETTER ***/

/**
 * @brief MovieSearch::scraperId
 * @return Current scraper Id
 */
QString MovieSearch::scraperId()
{
    return ui->movieSearchWidget->scraperId();
}

/**
 * @brief MovieSearch::scraperId
 * @return Scraper id of the movie last clicked in result table
 */
QString MovieSearch::scraperMovieId()
{
    return ui->movieSearchWidget->scraperMovieId();
}

/**
 * @brief MovieSearch::infosToLoad
 * @return List of infos to load from the scraper
 */
QList<int> MovieSearch::infosToLoad()
{
    return ui->movieSearchWidget->infosToLoad();
}

QMap<ScraperInterface*, QString> MovieSearch::customScraperIds()
{
    return ui->movieSearchWidget->customScraperIds();
}
