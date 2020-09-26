#include "TvShowSearchEpisode.h"
#include "ui_TvShowSearchEpisode.h"

#include "globals/Manager.h"
#include "scrapers/tv_show/TvScraperInterface.h"

#include <QDebug>

TvShowSearchEpisode::TvShowSearchEpisode(QWidget* parent) : QWidget(parent), ui(new Ui::TvShowSearchEpisode)
{
    ui->setupUi(this);

    ui->results->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->searchString->setType(MyLineEdit::TypeLoading);

    connect(Manager::instance()->scrapers().tvScrapers().at(0),
        &TvScraperInterface::sigSearchDone,
        this,
        &TvShowSearchEpisode::onShowResults);
    connect(ui->searchString, &QLineEdit::returnPressed, this, &TvShowSearchEpisode::onSearch);
    connect(ui->results, &QTableWidget::itemClicked, this, &TvShowSearchEpisode::onResultClicked);

    ui->chkCertification->setMyData(static_cast<int>(ShowScraperInfo::Certification));
    ui->chkDirector->setMyData(static_cast<int>(ShowScraperInfo::Director));
    ui->chkFirstAired->setMyData(static_cast<int>(ShowScraperInfo::FirstAired));
    ui->chkNetwork->setMyData(static_cast<int>(ShowScraperInfo::Network));
    ui->chkOverview->setMyData(static_cast<int>(ShowScraperInfo::Overview));
    ui->chkRating->setMyData(static_cast<int>(ShowScraperInfo::Rating));
    ui->chkThumbnail->setMyData(static_cast<int>(ShowScraperInfo::Thumbnail));
    ui->chkTitle->setMyData(static_cast<int>(ShowScraperInfo::Title));
    ui->chkWriter->setMyData(static_cast<int>(ShowScraperInfo::Writer));

    for (MyCheckBox* box : ui->groupBox->findChildren<MyCheckBox*>()) {
        if (box->myData().toInt() > 0) {
            connect(box, &QAbstractButton::clicked, this, &TvShowSearchEpisode::onChkToggled);
        }
    }

    connect(ui->chkUnCheckAll, &QAbstractButton::clicked, this, &TvShowSearchEpisode::onChkAllToggled);
}

TvShowSearchEpisode::~TvShowSearchEpisode()
{
    delete ui;
}

TvDbId TvShowSearchEpisode::scraperId()
{
    return m_scraperId;
}

void TvShowSearchEpisode::onChkToggled()
{
    m_infosToLoad.clear();
    bool allToggled = true;
    for (auto box : ui->groupBox->findChildren<MyCheckBox*>()) {
        if (box->isChecked() && box->myData().toInt() > 0 && box->isEnabled()) {
            m_infosToLoad.insert(ShowScraperInfo(box->myData().toInt()));
        }
        if (!box->isChecked() && box->myData().toInt() > 0 && box->isEnabled()) {
            allToggled = false;
        }
    }

    ui->chkUnCheckAll->setChecked(allToggled);

    Settings::instance()->setScraperInfos(QString::number(4), m_infosToLoad);
}

void TvShowSearchEpisode::onChkAllToggled()
{
    bool checked = ui->chkUnCheckAll->isChecked();
    for (MyCheckBox* box : ui->groupBox->findChildren<MyCheckBox*>()) {
        if (box->myData().toInt() > 0 && box->isEnabled()) {
            box->setChecked(checked);
        }
    }
    onChkToggled();
}

QSet<ShowScraperInfo> TvShowSearchEpisode::infosToLoad()
{
    return m_infosToLoad;
}

void TvShowSearchEpisode::onResultClicked(QTableWidgetItem* item)
{
    m_scraperId = TvDbId(item->data(Qt::UserRole).toString());
    emit sigResultClicked();
}

void TvShowSearchEpisode::search(QString searchString, TvDbId id)
{
    if (id.isValid()) {
        ui->searchString->setText(id.withPrefix());
    } else {
        ui->searchString->setText(searchString);
    }
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
    Manager::instance()->scrapers().tvScrapers().at(0)->search(ui->searchString->text());
}

void TvShowSearchEpisode::onShowResults(QVector<ScraperSearchResult> results)
{
    ui->searchString->setLoading(false);
    ui->searchString->setFocus();
    for (const ScraperSearchResult& result : results) {
        auto* item = new QTableWidgetItem(QString("%1 (%2)").arg(result.name).arg(result.released.toString("yyyy")));
        item->setData(Qt::UserRole, result.id);
        int row = ui->results->rowCount();
        ui->results->insertRow(row);
        ui->results->setItem(row, 0, item);
    }
}
