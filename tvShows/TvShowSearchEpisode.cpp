#include "TvShowSearchEpisode.h"
#include "ui_TvShowSearchEpisode.h"

#include <QDebug>

#include "globals/Manager.h"

TvShowSearchEpisode::TvShowSearchEpisode(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TvShowSearchEpisode)
{
    ui->setupUi(this);

    ui->results->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->searchString->setType(MyLineEdit::TypeLoading);

    connect(Manager::instance()->tvScrapers().at(0), SIGNAL(sigSearchDone(QList<ScraperSearchResult>)), this, SLOT(onShowResults(QList<ScraperSearchResult>)));
    connect(ui->searchString, SIGNAL(returnPressed()), this, SLOT(onSearch()));
    connect(ui->results, SIGNAL(itemClicked(QTableWidgetItem*)), this, SLOT(onResultClicked(QTableWidgetItem*)));

    ui->chkCertification->setMyData(TvShowScraperInfos::Certification);
    ui->chkDirector->setMyData(TvShowScraperInfos::Director);
    ui->chkFirstAired->setMyData(TvShowScraperInfos::FirstAired);
    ui->chkNetwork->setMyData(TvShowScraperInfos::Network);
    ui->chkOverview->setMyData(TvShowScraperInfos::Overview);
    ui->chkRating->setMyData(TvShowScraperInfos::Rating);
    ui->chkThumbnail->setMyData(TvShowScraperInfos::Thumbnail);
    ui->chkTitle->setMyData(TvShowScraperInfos::Title);
    ui->chkWriter->setMyData(TvShowScraperInfos::Writer);

    foreach (MyCheckBox *box, ui->groupBox->findChildren<MyCheckBox*>()) {
        if (box->myData().toInt() > 0)
            connect(box, SIGNAL(clicked()), this, SLOT(onChkToggled()));
    }

    connect(ui->chkUnCheckAll, SIGNAL(clicked()), this, SLOT(onChkAllToggled()));
}

TvShowSearchEpisode::~TvShowSearchEpisode()
{
    delete ui;
}

QString TvShowSearchEpisode::scraperId()
{
    return m_scraperId;
}

void TvShowSearchEpisode::onChkToggled()
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

    Settings::instance()->setScraperInfos(WidgetTvShows, QString::number(4), m_infosToLoad);
}

void TvShowSearchEpisode::onChkAllToggled()
{
    bool checked = ui->chkUnCheckAll->isChecked();
    foreach (MyCheckBox *box, ui->groupBox->findChildren<MyCheckBox*>()) {
        if (box->myData().toInt() > 0 && box->isEnabled())
            box->setChecked(checked);
    }
    onChkToggled();
}

QList<int> TvShowSearchEpisode::infosToLoad()
{
    return m_infosToLoad;
}

void TvShowSearchEpisode::onResultClicked(QTableWidgetItem *item)
{
    m_scraperId = item->data(Qt::UserRole).toString();
    emit sigResultClicked();
}

void TvShowSearchEpisode::search(QString searchString, QString id)
{
    if (!id.isEmpty())
        ui->searchString->setText("id" + id);
    else
        ui->searchString->setText(searchString);
    onChkToggled();
    onSearch();
}

void TvShowSearchEpisode::clear()
{
    ui->results->clearContents();
    ui->results->setRowCount(0);
}

void TvShowSearchEpisode::onSearch()
{
    clear();
    ui->searchString->setLoading(true);
    Manager::instance()->tvScrapers().at(0)->search(ui->searchString->text());
}

void TvShowSearchEpisode::onShowResults(QList<ScraperSearchResult> results)
{
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
