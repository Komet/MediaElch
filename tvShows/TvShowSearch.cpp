#include "TvShowSearch.h"
#include "ui_TvShowSearch.h"

#include "globals/Manager.h"
#include "smallWidgets/MyCheckBox.h"

/**
 * @brief TvShowSearch::TvShowSearch
 * @param parent
 */
TvShowSearch::TvShowSearch(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TvShowSearch)
{
    ui->setupUi(this);
#if QT_VERSION >= 0x050000
    ui->results->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
#else
    ui->results->verticalHeader()->setResizeMode(QHeaderView::ResizeToContents);
#endif
    ui->searchString->setType(MyLineEdit::TypeLoading);

#ifdef Q_OS_MAC
    setWindowFlags((windowFlags() & ~Qt::WindowType_Mask) | Qt::Sheet);
    setStyleSheet(styleSheet() + " #TvShowSearch { border: 1px solid rgba(0, 0, 0, 100); border-top: none; }");
#else
    setWindowFlags((windowFlags() & ~Qt::WindowType_Mask) | Qt::Dialog);
#endif

    connect(Manager::instance()->tvScrapers().at(0), SIGNAL(sigSearchDone(QList<ScraperSearchResult>)), this, SLOT(onShowResults(QList<ScraperSearchResult>)));
    connect(ui->searchString, SIGNAL(returnPressed()), this, SLOT(onSearch()));
    connect(ui->results, SIGNAL(itemClicked(QTableWidgetItem*)), this, SLOT(onResultClicked(QTableWidgetItem*)));
    connect(ui->buttonClose, SIGNAL(clicked()), this, SLOT(reject()));

    ui->chkActors->setMyData(TvShowScraperInfos::Actors);
    ui->chkBanner->setMyData(TvShowScraperInfos::Banner);
    ui->chkCertification->setMyData(TvShowScraperInfos::Certification);
    ui->chkDirector->setMyData(TvShowScraperInfos::Director);
    ui->chkFanart->setMyData(TvShowScraperInfos::Fanart);
    ui->chkFirstAired->setMyData(TvShowScraperInfos::FirstAired);
    ui->chkGenres->setMyData(TvShowScraperInfos::Genres);
    ui->chkNetwork->setMyData(TvShowScraperInfos::Network);
    ui->chkOverview->setMyData(TvShowScraperInfos::Overview);
    ui->chkPoster->setMyData(TvShowScraperInfos::Poster);
    ui->chkRating->setMyData(TvShowScraperInfos::Rating);
    ui->chkSeasonEpisode->setMyData(TvShowScraperInfos::SeasonEpisode);
    ui->chkSeasonPoster->setMyData(TvShowScraperInfos::SeasonPoster);
    ui->chkThumbnail->setMyData(TvShowScraperInfos::Thumbnail);
    ui->chkTitle->setMyData(TvShowScraperInfos::Title);
    ui->chkWriter->setMyData(TvShowScraperInfos::Writer);

    foreach (MyCheckBox *box, ui->groupBox->findChildren<MyCheckBox*>()) {
        if (box->myData().toInt() > 0)
            connect(box, SIGNAL(clicked()), this, SLOT(onChkToggled()));
    }

    connect(ui->chkUnCheckAll, SIGNAL(clicked()), this, SLOT(onChkAllToggled()));
}

/**
 * @brief TvShowSearch::~TvShowSearch
 */
TvShowSearch::~TvShowSearch()
{
    delete ui;
}

/**
 * @brief Returns the instance of the dialog
 * @param parent Parent widget (used only the first time for constructing)
 * @return Instance of the dialog
 */
TvShowSearch* TvShowSearch::instance(QWidget *parent)
{
    static TvShowSearch *m_instance = 0;
    if (m_instance == 0) {
        m_instance = new TvShowSearch(parent);
    }
    return m_instance;
}

/**
 * @brief Adjusts size and executes the dialog
 * @param searchString String to search for
 * @return Result of QDialog::exec
 */
int TvShowSearch::exec(QString searchString)
{
    qDebug() << "Entered, searchString=" << searchString;
    QSize newSize;
    newSize.setHeight(parentWidget()->size().height()-200);
    newSize.setWidth(qMin(600, parentWidget()->size().width()-400));
    resize(newSize);

    ui->searchString->setText(searchString);
    onSearch();
    onChkToggled();
    return QDialog::exec();
}

/**
 * @brief Clears the widgets contents
 */
void TvShowSearch::clear()
{
    qDebug() << "Entered";
    ui->results->clearContents();
    ui->results->setRowCount(0);
}

/**
 * @brief Tells the current scraper to search
 */
void TvShowSearch::onSearch()
{
    qDebug() << "Entered, with" << ui->searchString->text();
    clear();
    ui->searchString->setLoading(true);
    Manager::instance()->tvScrapers().at(0)->search(ui->searchString->text());
}

/**
 * @brief Displays the results from the scraper
 * @param results List of results
 */
void TvShowSearch::onShowResults(QList<ScraperSearchResult> results)
{
    qDebug() << "Entered, size of results=" << results.count();
    ui->searchString->setLoading(false);
    ui->searchString->setFocus();
    foreach (const ScraperSearchResult &result, results) {
        QTableWidgetItem *item = new QTableWidgetItem(QString("%1 (%2)").arg(result.name).arg(result.released.toString("yyyy")));
        item->setData(Qt::UserRole, result.id);
        int row = ui->results->rowCount();
        ui->results->insertRow(row);
        ui->results->setItem(row, 0, item);
    }
}

/**
 * @brief Stores the clicked id and accepts the dialog
 * @param item Item which was clicked
 */
void TvShowSearch::onResultClicked(QTableWidgetItem *item)
{
    qDebug() << "Entered";
    m_scraperId = item->data(Qt::UserRole).toString();
    qDebug() << "m_scraperId=" << m_scraperId;
    this->accept();
}

/**
 * @brief Toggles the visibility of the "Update all episodes" checkbox
 * @param visible Visibility
 */
void TvShowSearch::setChkUpdateAllVisible(bool visible)
{
    ui->chkUpdateAllEpisodes->setVisible(visible);
}

/*** GETTER ***/

/**
 * @brief Returns the id of the current scraper
 * @return Id of the current scraper
 */
QString TvShowSearch::scraperId()
{
    qDebug() << "Entered, m_scraperId" << m_scraperId;
    return m_scraperId;
}

/**
 * @brief Returns the state of the "Update all episodes" checkbox
 * @return Is update all episodes checked
 */
bool TvShowSearch::updateAll()
{
    qDebug() << "Entered, updateAll=" << ui->chkUpdateAllEpisodes->isChecked();
    return ui->chkUpdateAllEpisodes->isChecked();
}

void TvShowSearch::onChkToggled()
{
    m_infosToLoad.clear();
    bool allToggled = true;
    foreach (MyCheckBox *box, ui->groupBox->findChildren<MyCheckBox*>()) {
        if (box->isChecked() && box->myData().toInt() > 0)
            m_infosToLoad.append(box->myData().toInt());
        if (!box->isChecked() && box->myData().toInt() > 0)
            allToggled = false;
    }

    ui->chkUnCheckAll->setChecked(allToggled);
}

void TvShowSearch::onChkAllToggled()
{
    bool checked = ui->chkUnCheckAll->isChecked();
    foreach (MyCheckBox *box, ui->groupBox->findChildren<MyCheckBox*>()) {
        if (box->myData().toInt() > 0)
            box->setChecked(checked);
    }
    onChkToggled();
}

QList<int> TvShowSearch::infosToLoad()
{
    return m_infosToLoad;
}
