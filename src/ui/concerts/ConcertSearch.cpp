#include "ConcertSearch.h"
#include "ui_ConcertSearch.h"

#include <QDebug>

#include "globals/Manager.h"

ConcertSearch::ConcertSearch(QWidget* parent) : QDialog(parent), ui(new Ui::ConcertSearch)
{
    ui->setupUi(this);

#ifdef Q_OS_MAC
    setWindowFlags((windowFlags() & ~Qt::WindowType_Mask) | Qt::Sheet);
    setStyleSheet(styleSheet() + " #ConcertSearch { border: 1px solid rgba(0, 0, 0, 100); border-top: none; }");
#else
    setWindowFlags((windowFlags() & ~Qt::WindowType_Mask) | Qt::Dialog);
#endif

    connect(ui->buttonClose, &QAbstractButton::clicked, this, &QDialog::reject);
    connect(ui->concertSearchWidget, &ConcertSearchWidget::sigResultClicked, this, &QDialog::accept);
}

ConcertSearch::~ConcertSearch()
{
    delete ui;
}

/**
 * \brief Executes the search dialog
 * \param searchString Concert name/search string
 * \return Result of QDialog::exec
 * \see ConcertSearch::search
 */
int ConcertSearch::execWithSearch(QString searchString)
{
    QSize newSize;
    newSize.setHeight(parentWidget()->size().height() - 200);
    newSize.setWidth(qMin(600, parentWidget()->size().width() - 400));
    resize(newSize);

    ui->concertSearchWidget->search(searchString);
    return QDialog::exec();
}

/**
 * \brief ConcertSearch::scraperNo
 * \return Current scraper number
 */
mediaelch::scraper::ConcertScraper* ConcertSearch::scraper()
{
    return ui->concertSearchWidget->scraper();
}

/**
 * \brief ConcertSearch::scraperId
 * \return Scraper id of the concert last clicked in result table
 */
QString ConcertSearch::concertIdentifier()
{
    return ui->concertSearchWidget->concertIdentifier();
}

/**
 * \brief ConcertSearch::infosToLoad
 * \return List of infos to load from the scraper
 */
QSet<ConcertScraperInfo> ConcertSearch::infosToLoad()
{
    return ui->concertSearchWidget->concertDetailsToLoad();
}
