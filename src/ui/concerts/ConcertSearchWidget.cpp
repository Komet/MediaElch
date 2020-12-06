#include "ConcertSearchWidget.h"
#include "ui_ConcertSearchWidget.h"

#include "scrapers/concert/ConcertScraperInterface.h"

#include <QDebug>

#include "globals/Manager.h"

ConcertSearchWidget::ConcertSearchWidget(QWidget* parent) : QWidget(parent), ui(new Ui::ConcertSearchWidget)
{
    ui->setupUi(this);

    ui->results->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->searchString->setType(MyLineEdit::TypeLoading);

    for (ConcertScraperInterface* scraper : Manager::instance()->scrapers().concertScrapers()) {
        ui->comboScraper->addItem(scraper->name(), Manager::instance()->scrapers().concertScrapers().indexOf(scraper));
        connect(scraper, &ConcertScraperInterface::searchDone, this, &ConcertSearchWidget::showResults);
    }

    connect(ui->comboScraper,
        elchOverload<int>(&QComboBox::currentIndexChanged),
        this,
        &ConcertSearchWidget::searchByComboIndex);

    connect(ui->searchString, &MyLineEdit::returnPressed, this, [this]() {
        const int index = ui->comboScraper->currentIndex();
        searchByComboIndex(index);
    });

    connect(ui->results, &QTableWidget::itemClicked, this, &ConcertSearchWidget::resultClicked);

    ui->chkBackdrop->setMyData(static_cast<int>(ConcertScraperInfo::Backdrop));
    ui->chkCertification->setMyData(static_cast<int>(ConcertScraperInfo::Certification));
    ui->chkExtraArts->setMyData(static_cast<int>(ConcertScraperInfo::ExtraArts));
    ui->chkGenres->setMyData(static_cast<int>(ConcertScraperInfo::Genres));
    ui->chkOverview->setMyData(static_cast<int>(ConcertScraperInfo::Overview));
    ui->chkPoster->setMyData(static_cast<int>(ConcertScraperInfo::Poster));
    ui->chkRating->setMyData(static_cast<int>(ConcertScraperInfo::Rating));
    ui->chkReleased->setMyData(static_cast<int>(ConcertScraperInfo::Released));
    ui->chkRuntime->setMyData(static_cast<int>(ConcertScraperInfo::Runtime));
    ui->chkTagline->setMyData(static_cast<int>(ConcertScraperInfo::Tagline));
    ui->chkTitle->setMyData(static_cast<int>(ConcertScraperInfo::Title));
    ui->chkTrailer->setMyData(static_cast<int>(ConcertScraperInfo::Trailer));

    for (MyCheckBox* box : ui->groupBox->findChildren<MyCheckBox*>()) {
        if (box->myData().toInt() > 0) {
            connect(box, &QAbstractButton::clicked, this, &ConcertSearchWidget::chkToggled);
        }
    }
    connect(ui->chkUnCheckAll, &QAbstractButton::clicked, this, &ConcertSearchWidget::chkAllToggled);
}

ConcertSearchWidget::~ConcertSearchWidget()
{
    delete ui;
}

void ConcertSearchWidget::search(QString searchString)
{
    ui->searchString->setText(searchString.replace(".", " ").trimmed());
    const int index = ui->comboScraper->currentIndex();
    searchByComboIndex(index);
}

void ConcertSearchWidget::clear()
{
    ui->results->clearContents();
    ui->results->setRowCount(0);
}

void ConcertSearchWidget::searchByComboIndex(int comboScraperIndex)
{
    qDebug() << "[ConcertSearchWidget] Start search";
    if (comboScraperIndex < 0 || comboScraperIndex >= Manager::instance()->scrapers().concertScrapers().size()) {
        return;
    }
    m_scraperNo = ui->comboScraper->itemData(comboScraperIndex, Qt::UserRole).toInt();
    setCheckBoxesEnabled(Manager::instance()->scrapers().concertScrapers().at(m_scraperNo)->scraperSupports());
    clear();
    ui->comboScraper->setEnabled(false);
    ui->searchString->setLoading(true);
    Manager::instance()->scrapers().concertScrapers().at(m_scraperNo)->search(ui->searchString->text().trimmed());
}

void ConcertSearchWidget::showResults(QVector<ScraperSearchResult> results)
{
    qDebug() << "Entered, size of results=" << results.count();
    ui->comboScraper->setEnabled(true);
    ui->searchString->setLoading(false);
    ui->searchString->setFocus();
    for (const ScraperSearchResult& result : results) {
        QString name = result.name;
        if (result.released.isValid()) {
            name.append(QString(" (%1)").arg(result.released.toString("yyyy")));
        }
        auto* item = new QTableWidgetItem(name);
        item->setData(Qt::UserRole, result.id);
        int row = ui->results->rowCount();
        ui->results->insertRow(row);
        ui->results->setItem(row, 0, item);
    }
}

void ConcertSearchWidget::resultClicked(QTableWidgetItem* item)
{
    m_scraperId = TmdbId(item->data(Qt::UserRole).toString());
    emit sigResultClicked();
}

void ConcertSearchWidget::chkToggled()
{
    m_infosToLoad.clear();
    bool allToggled = true;
    for (MyCheckBox* box : ui->groupBox->findChildren<MyCheckBox*>()) {
        if (box->isChecked() && box->myData().toInt() > 0 && box->isEnabled()) {
            m_infosToLoad.insert(ConcertScraperInfo(box->myData().toInt()));
        }
        if (!box->isChecked() && box->myData().toInt() > 0 && box->isEnabled()) {
            allToggled = false;
        }
    }
    ui->chkUnCheckAll->setChecked(allToggled);

    int scraperNo = ui->comboScraper->itemData(ui->comboScraper->currentIndex(), Qt::UserRole).toInt();
    Settings::instance()->setScraperInfos(QString::number(scraperNo), m_infosToLoad);
}

void ConcertSearchWidget::chkAllToggled(bool toggled)
{
    for (MyCheckBox* box : ui->groupBox->findChildren<MyCheckBox*>()) {
        if (box->myData().toInt() > 0 && box->isEnabled()) {
            box->setChecked(toggled);
        }
    }
    chkToggled();
}

int ConcertSearchWidget::scraperNo() const
{
    qDebug() << "Entered, m_scraperNo=" << m_scraperNo;
    return m_scraperNo;
}

TmdbId ConcertSearchWidget::scraperId()
{
    qDebug() << "Entered, m_scraperId=" << m_scraperId;
    return m_scraperId;
}

QSet<ConcertScraperInfo> ConcertSearchWidget::infosToLoad()
{
    return m_infosToLoad;
}

void ConcertSearchWidget::setCheckBoxesEnabled(QSet<ConcertScraperInfo> scraperSupports)
{
    int scraperNo = ui->comboScraper->itemData(ui->comboScraper->currentIndex(), Qt::UserRole).toInt();
    QSet<ConcertScraperInfo> infos = Settings::instance()->scraperInfos<ConcertScraperInfo>(QString::number(scraperNo));

    for (auto box : ui->groupBox->findChildren<MyCheckBox*>()) {
        box->setEnabled(scraperSupports.contains(ConcertScraperInfo(box->myData().toInt())));
        box->setChecked((infos.contains(ConcertScraperInfo(box->myData().toInt())) || infos.isEmpty())
                        && scraperSupports.contains(ConcertScraperInfo(box->myData().toInt())));
    }
    chkToggled();
}
