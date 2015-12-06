#include "MusicSearchWidget.h"
#include "ui_MusicSearchWidget.h"

#include <QDebug>
#include "globals/Manager.h"
#include "smallWidgets/MyLabel.h"

MusicSearchWidget::MusicSearchWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MusicSearchWidget)
{
    ui->setupUi(this);

    ui->results->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->searchString->setType(MyLineEdit::TypeLoading);

    foreach (MusicScraperInterface *scraper, Manager::instance()->musicScrapers()) {
        ui->comboScraper->addItem(scraper->name(), Manager::instance()->musicScrapers().indexOf(scraper));
        connect(scraper, SIGNAL(sigSearchDone(QList<ScraperSearchResult>)), this, SLOT(showResults(QList<ScraperSearchResult>)));
    }

    connect(ui->comboScraper, SIGNAL(currentIndexChanged(int)), this, SLOT(search()));
    connect(ui->searchString, SIGNAL(returnPressed()), this, SLOT(search()));
    connect(ui->results, SIGNAL(itemClicked(QTableWidgetItem*)), this, SLOT(resultClicked(QTableWidgetItem*)));

    ui->chkName->setMyData(MusicScraperInfos::Name);
    ui->chkBorn->setMyData(MusicScraperInfos::Born);
    ui->chkFormed->setMyData(MusicScraperInfos::Formed);
    ui->chkYearsActive->setMyData(MusicScraperInfos::YearsActive);
    ui->chkDisbanded->setMyData(MusicScraperInfos::Disbanded);
    ui->chkDied->setMyData(MusicScraperInfos::Died);
    ui->chkBiography->setMyData(MusicScraperInfos::Biography);
    ui->chkArtist->setMyData(MusicScraperInfos::Artist);
    ui->chkLabel->setMyData(MusicScraperInfos::Label);
    ui->chkReview->setMyData(MusicScraperInfos::Review);
    ui->chkYear->setMyData(MusicScraperInfos::Year);
    ui->chkRating->setMyData(MusicScraperInfos::Rating);
    ui->chkReleaseDate->setMyData(MusicScraperInfos::ReleaseDate);
    ui->chkTitle->setMyData(MusicScraperInfos::Title);
    ui->chkGenres->setMyData(MusicScraperInfos::Genres);
    ui->chkStyles->setMyData(MusicScraperInfos::Styles);
    ui->chkMoods->setMyData(MusicScraperInfos::Moods);
    ui->chkThumb->setMyData(MusicScraperInfos::Thumb);
    ui->chkFanart->setMyData(MusicScraperInfos::Fanart);
    ui->chkExtraFanarts->setMyData(MusicScraperInfos::ExtraFanarts);
    ui->chkLogo->setMyData(MusicScraperInfos::Logo);
    ui->chkCover->setMyData(MusicScraperInfos::Cover);
    ui->chkCdArt->setMyData(MusicScraperInfos::CdArt);
    ui->chkDiscography->setMyData(MusicScraperInfos::Discography);

    foreach (MyCheckBox *box, ui->groupBox->findChildren<MyCheckBox*>()) {
        if (box->myData().toInt() > 0)
            connect(box, SIGNAL(clicked()), this, SLOT(chkToggled()));
    }
    connect(ui->chkUnCheckAll, SIGNAL(clicked(bool)), this, SLOT(chkAllToggled(bool)));

    m_signalMapper = new QSignalMapper(ui->results);
    connect(m_signalMapper, SIGNAL(mapped(int)), this, SLOT(resultClicked(int)));
}

MusicSearchWidget::~MusicSearchWidget()
{
    delete ui;
}

void MusicSearchWidget::search(QString searchString)
{
    ui->searchString->setText(searchString.replace(".", " "));
    search();
}

void MusicSearchWidget::clear()
{
    ui->results->clearContents();
    ui->results->setRowCount(0);
}

void MusicSearchWidget::search()
{
    int index = ui->comboScraper->currentIndex();
    if (index < 0 || index >= Manager::instance()->musicScrapers().size())
        return;
    m_scraperNo = ui->comboScraper->itemData(index, Qt::UserRole).toInt();
    setChkBoxesEnabled(Manager::instance()->musicScrapers().at(m_scraperNo)->scraperSupports());
    clear();
    ui->comboScraper->setEnabled(false);
    ui->searchString->setLoading(true);
    if (m_type == "artist")
        Manager::instance()->musicScrapers().at(m_scraperNo)->searchArtist(ui->searchString->text());
    else if (m_type == "album")
        Manager::instance()->musicScrapers().at(m_scraperNo)->searchAlbum(m_artistName, ui->searchString->text());
}

void MusicSearchWidget::showResults(QList<ScraperSearchResult> results)
{
    ui->comboScraper->setEnabled(true);
    ui->searchString->setLoading(false);
    ui->searchString->setFocus();

    foreach (const ScraperSearchResult &result, results) {
        MyLabel *label = new MyLabel(ui->results);
        QString name = result.name;
        if (result.released.isValid())
            name.append(QString(" (%1)").arg(result.released.toString("yyyy")));

        label->setText(name + "<br /><span style=\"color: #999999;\">" + result.id + "</span>");
        label->setMargin(8);

        QTableWidgetItem *item = new QTableWidgetItem;
        item->setData(Qt::UserRole, result.id);
        item->setData(Qt::UserRole+1, result.id2);
        int row = ui->results->rowCount();
        ui->results->insertRow(row);
        ui->results->setItem(row, 0, item);
        ui->results->setCellWidget(row, 0, label);

        connect(label, SIGNAL(clicked()), m_signalMapper, SLOT(map()));
        m_signalMapper->setMapping(label, row);
    }
}

void MusicSearchWidget::resultClicked(QTableWidgetItem *item)
{
    m_scraperId = item->data(Qt::UserRole).toString();
    m_scraperId2 = item->data(Qt::UserRole+1).toString();
    emit sigResultClicked();
}

void MusicSearchWidget::resultClicked(int row)
{
    if (row < 0 || row >= ui->results->rowCount())
        return;
    resultClicked(ui->results->item(row, 0));
}

void MusicSearchWidget::chkToggled()
{
    m_infosToLoad.clear();
    bool allToggled = true;
    foreach (MyCheckBox *box, ui->groupBox->findChildren<MyCheckBox*>()) {
        if (box->isChecked() && box->myData().toInt() > 0 && box->isEnabled() && !box->isHidden())
            m_infosToLoad.append(box->myData().toInt());
        if (!box->isChecked() && box->myData().toInt() > 0 && box->isEnabled() && !box->isHidden())
            allToggled = false;
    }
    ui->chkUnCheckAll->setChecked(allToggled);

    int scraperNo = ui->comboScraper->itemData(ui->comboScraper->currentIndex(), Qt::UserRole).toInt();
    Settings::instance()->setScraperInfos(WidgetMusic, m_type + "/" + QString::number(scraperNo), m_infosToLoad);
}

void MusicSearchWidget::chkAllToggled(bool toggled)
{
    foreach (MyCheckBox *box, ui->groupBox->findChildren<MyCheckBox*>()) {
        if (box->myData().toInt() > 0 && box->isEnabled() && !box->isHidden())
            box->setChecked(toggled);
    }
    chkToggled();
}

int MusicSearchWidget::scraperNo()
{
    return m_scraperNo;
}

QString MusicSearchWidget::scraperId()
{
    return m_scraperId;
}

QString MusicSearchWidget::scraperId2()
{
    return m_scraperId2;
}

QList<int> MusicSearchWidget::infosToLoad()
{
    return m_infosToLoad;
}

void MusicSearchWidget::setChkBoxesEnabled(QList<int> scraperSupports)
{
    int scraperNo = ui->comboScraper->itemData(ui->comboScraper->currentIndex(), Qt::UserRole).toInt();
    QList<int> infos = Settings::instance()->scraperInfos(WidgetMusic, m_type + "/" + QString::number(scraperNo));

    foreach (MyCheckBox *box, ui->groupBox->findChildren<MyCheckBox*>()) {
        box->setEnabled(scraperSupports.contains(box->myData().toInt()));
        box->setChecked((infos.contains(box->myData().toInt()) || infos.isEmpty()) && scraperSupports.contains(box->myData().toInt()));
    }
    chkToggled();
}

void MusicSearchWidget::setType(const QString &type)
{
    m_type = type;
    foreach (MyCheckBox *box, ui->groupBox->findChildren<MyCheckBox*>())
        box->setVisible(box->property("type").toString() == "both" || box->property("type").toString() == type);
}

void MusicSearchWidget::setArtistName(const QString &artistName)
{
    m_artistName = artistName;
}
