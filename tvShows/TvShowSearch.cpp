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
    ui->results->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
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
    connect(ui->comboUpdate, SIGNAL(currentIndexChanged(int)), this, SLOT(onComboIndexChanged()));
    connect(ui->chkDvdOrder, SIGNAL(clicked()), this, SLOT(onChkDvdOrderToggled()));

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
    ui->chkSeasonPoster->setMyData(TvShowScraperInfos::SeasonPoster);
    ui->chkSeasonBackdrop->setMyData(TvShowScraperInfos::SeasonBackdrop);
    ui->chkSeasonBanner->setMyData(TvShowScraperInfos::SeasonBanner);
    ui->chkThumbnail->setMyData(TvShowScraperInfos::Thumbnail);
    ui->chkTitle->setMyData(TvShowScraperInfos::Title);
    ui->chkWriter->setMyData(TvShowScraperInfos::Writer);
    ui->chkExtraArts->setMyData(TvShowScraperInfos::ExtraArts);
    ui->chkRuntime->setMyData(TvShowScraperInfos::Runtime);
    ui->chkStatus->setMyData(TvShowScraperInfos::Status);

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
int TvShowSearch::exec(QString searchString, QString id)
{
    QSize newSize;
    newSize.setHeight(parentWidget()->size().height()-200);
    newSize.setWidth(qMin(600, parentWidget()->size().width()-400));
    resize(newSize);

    if (!id.isEmpty())
        ui->searchString->setText("id" + id);
    else
        ui->searchString->setText(searchString.replace(".", " "));

    ui->chkDvdOrder->setChecked(Settings::instance()->tvShowDvdOrder());

    onChkToggled();
    onSearch();
    return QDialog::exec();
}

int TvShowSearch::exec()
{
    return 0;
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

void TvShowSearch::setSearchType(TvShowType type)
{
    m_searchType = type;
    if (type == TypeTvShow) {
        ui->comboUpdate->setVisible(true);
        ui->comboUpdate->setCurrentIndex(Settings::instance()->tvShowUpdateOption());
        onComboIndexChanged();
    } else if (type == TypeEpisode) {
        ui->comboUpdate->setVisible(false);
        ui->comboUpdate->setCurrentIndex(4);
        onComboIndexChanged();
    }
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

void TvShowSearch::onChkToggled()
{
    m_infosToLoad.clear();
    bool allToggled = true;
    foreach (MyCheckBox *box, ui->groupBox->findChildren<MyCheckBox*>()) {
        if (box->isChecked() && box->myData().toInt() > 0 && box->isEnabled())
            m_infosToLoad.append(box->myData().toInt());
        if (!box->isChecked() && box->myData().toInt() > 0 && box->isEnabled())
            allToggled = false;
    }

    ui->chkUnCheckAll->setChecked(allToggled);

    int scraperNo = ui->comboUpdate->currentIndex();
    if (m_searchType == TypeEpisode)
        scraperNo = 4;
    Settings::instance()->setScraperInfos(WidgetTvShows, QString::number(scraperNo), m_infosToLoad);
}

void TvShowSearch::onChkAllToggled()
{
    bool checked = ui->chkUnCheckAll->isChecked();
    foreach (MyCheckBox *box, ui->groupBox->findChildren<MyCheckBox*>()) {
        if (box->myData().toInt() > 0 && box->isEnabled())
            box->setChecked(checked);
    }
    onChkToggled();
}

QList<int> TvShowSearch::infosToLoad()
{
    return m_infosToLoad;
}

TvShowUpdateType TvShowSearch::updateType()
{
    if (ui->comboUpdate->currentIndex() == 0)
        return UpdateShow;
    if (ui->comboUpdate->currentIndex() == 1)
        return UpdateShowAndNewEpisodes;
    if (ui->comboUpdate->currentIndex() == 2)
        return UpdateShowAndAllEpisodes;
    if (ui->comboUpdate->currentIndex() == 3)
        return UpdateNewEpisodes;
    return UpdateAllEpisodes;
}

void TvShowSearch::onComboIndexChanged()
{
    int scraperNo = ui->comboUpdate->currentIndex();
    if (m_searchType == TypeEpisode)
        scraperNo = 4;
    else
        Settings::instance()->setTvShowUpdateOption(ui->comboUpdate->currentIndex());
    QList<int> infos = Settings::instance()->scraperInfos(WidgetTvShows, QString::number(scraperNo));

    TvShowUpdateType type = updateType();
    if (type == UpdateShow) {
        ui->chkGenres->setEnabled(true);
        ui->chkActors->setEnabled(true);
        ui->chkSeasonPoster->setEnabled(true);
        ui->chkSeasonBackdrop->setEnabled(true);
        ui->chkSeasonBanner->setEnabled(true);
        ui->chkBanner->setEnabled(true);
        ui->chkFanart->setEnabled(true);
        ui->chkPoster->setEnabled(true);
        ui->chkExtraArts->setEnabled(true);
        ui->chkThumbnail->setEnabled(false);
        ui->chkDirector->setEnabled(false);
        ui->chkWriter->setEnabled(false);
        ui->chkRuntime->setEnabled(true);
        ui->chkStatus->setEnabled(true);
    } else if (type == UpdateShowAndAllEpisodes || type == UpdateShowAndNewEpisodes) {
        ui->chkGenres->setEnabled(true);
        ui->chkActors->setEnabled(true);
        ui->chkSeasonPoster->setEnabled(true);
        ui->chkSeasonBackdrop->setEnabled(true);
        ui->chkSeasonBanner->setEnabled(true);
        ui->chkBanner->setEnabled(true);
        ui->chkFanart->setEnabled(true);
        ui->chkPoster->setEnabled(true);
        ui->chkExtraArts->setEnabled(true);
        ui->chkThumbnail->setEnabled(true);
        ui->chkDirector->setEnabled(true);
        ui->chkWriter->setEnabled(true);
        ui->chkRuntime->setEnabled(true);
        ui->chkStatus->setEnabled(true);
    } else {
        ui->chkGenres->setEnabled(false);
        ui->chkActors->setEnabled(true);
        ui->chkSeasonPoster->setEnabled(false);
        ui->chkSeasonBackdrop->setEnabled(false);
        ui->chkSeasonBanner->setEnabled(false);
        ui->chkBanner->setEnabled(false);
        ui->chkFanart->setEnabled(false);
        ui->chkPoster->setEnabled(false);
        ui->chkExtraArts->setEnabled(false);
        ui->chkThumbnail->setEnabled(true);
        ui->chkDirector->setEnabled(true);
        ui->chkWriter->setEnabled(true);
        ui->chkRuntime->setEnabled(false);
        ui->chkStatus->setEnabled(false);
    }

    foreach (MyCheckBox *box, ui->groupBox->findChildren<MyCheckBox*>())
        box->setChecked((infos.contains(box->myData().toInt()) || infos.isEmpty()) && box->isEnabled());
    onChkToggled();
}

void TvShowSearch::onChkDvdOrderToggled()
{
    Settings::instance()->setTvShowDvdOrder(ui->chkDvdOrder->isChecked());
}
