#include "MusicSearchWidget.h"
#include "ui_MusicSearchWidget.h"

#include "globals/Manager.h"
#include "log/Log.h"
#include "scrapers/music/MusicScraper.h"
#include "ui/small_widgets/MyLabel.h"

MusicSearchWidget::MusicSearchWidget(QWidget* parent) : QWidget(parent), ui(new Ui::MusicSearchWidget)
{
    ui->setupUi(this);

    ui->results->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->searchString->setType(MyLineEdit::TypeLoading);

    int index = 0;
    for (auto* scraper : Manager::instance()->scrapers().musicScrapers()) {
        ui->comboScraper->addItem(scraper->meta().name, index);
        ++index;
    }

    connect(ui->comboScraper,
        elchOverload<int>(&QComboBox::currentIndexChanged), //
        this,
        &MusicSearchWidget::startSearchWithIndex);
    connect(ui->searchString, &MyLineEdit::returnPressed, this, &MusicSearchWidget::startSearch);
    connect(ui->results, &QTableWidget::itemClicked, this, &MusicSearchWidget::resultClicked);

    ui->chkName->setMyData(static_cast<int>(MusicScraperInfo::Name));
    ui->chkBorn->setMyData(static_cast<int>(MusicScraperInfo::Born));
    ui->chkFormed->setMyData(static_cast<int>(MusicScraperInfo::Formed));
    ui->chkYearsActive->setMyData(static_cast<int>(MusicScraperInfo::YearsActive));
    ui->chkDisbanded->setMyData(static_cast<int>(MusicScraperInfo::Disbanded));
    ui->chkDied->setMyData(static_cast<int>(MusicScraperInfo::Died));
    ui->chkBiography->setMyData(static_cast<int>(MusicScraperInfo::Biography));
    ui->chkArtist->setMyData(static_cast<int>(MusicScraperInfo::Artist));
    ui->chkLabel->setMyData(static_cast<int>(MusicScraperInfo::Label));
    ui->chkReview->setMyData(static_cast<int>(MusicScraperInfo::Review));
    ui->chkYear->setMyData(static_cast<int>(MusicScraperInfo::Year));
    ui->chkRating->setMyData(static_cast<int>(MusicScraperInfo::Rating));
    ui->chkReleaseDate->setMyData(static_cast<int>(MusicScraperInfo::ReleaseDate));
    ui->chkTitle->setMyData(static_cast<int>(MusicScraperInfo::Title));
    ui->chkGenres->setMyData(static_cast<int>(MusicScraperInfo::Genres));
    ui->chkStyles->setMyData(static_cast<int>(MusicScraperInfo::Styles));
    ui->chkMoods->setMyData(static_cast<int>(MusicScraperInfo::Moods));
    ui->chkThumb->setMyData(static_cast<int>(MusicScraperInfo::Thumb));
    ui->chkFanart->setMyData(static_cast<int>(MusicScraperInfo::Fanart));
    ui->chkExtraFanarts->setMyData(static_cast<int>(MusicScraperInfo::ExtraFanarts));
    ui->chkLogo->setMyData(static_cast<int>(MusicScraperInfo::Logo));
    ui->chkCover->setMyData(static_cast<int>(MusicScraperInfo::Cover));
    ui->chkCdArt->setMyData(static_cast<int>(MusicScraperInfo::CdArt));
    ui->chkDiscography->setMyData(static_cast<int>(MusicScraperInfo::Discography));

    for (MyCheckBox* box : ui->groupBox->findChildren<MyCheckBox*>()) {
        if (box->myData().toInt() > 0) {
            connect(box, &QAbstractButton::clicked, this, &MusicSearchWidget::chkToggled);
        }
    }
    connect(ui->chkUnCheckAll, &QAbstractButton::clicked, this, &MusicSearchWidget::chkAllToggled);

    m_signalMapper = new QSignalMapper(ui->results);
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
    connect(m_signalMapper, elchOverload<int>(&QSignalMapper::mapped), this, &MusicSearchWidget::resultClickedRow);
#else
    connect(m_signalMapper, &QSignalMapper::mappedInt, this, &MusicSearchWidget::resultClickedRow);
#endif
}

MusicSearchWidget::~MusicSearchWidget()
{
    delete ui;
}

void MusicSearchWidget::search(QString searchString)
{
    ui->searchString->setText(searchString.replace(".", " ").trimmed());
    startSearch();
}

void MusicSearchWidget::clear()
{
    ui->results->clearContents();
    ui->results->setRowCount(0);
}
void MusicSearchWidget::startSearch()
{
    const int index = ui->comboScraper->currentIndex();
    startSearchWithIndex(index);
}

void MusicSearchWidget::startSearchWithIndex(int index)
{
    if (index < 0 || index >= Manager::instance()->scrapers().musicScrapers().size()) {
        return;
    }
    m_scraperNo = ui->comboScraper->itemData(index, Qt::UserRole).toInt();
    setCheckBoxesEnabled(Manager::instance()->scrapers().musicScrapers().at(m_scraperNo)->meta().supportedDetails);
    clear();
    ui->comboScraper->setEnabled(false);
    ui->searchString->setLoading(true);

    auto* scraper = Manager::instance()->scrapers().musicScrapers().at(m_scraperNo);
    QString input = ui->searchString->text().trimmed();

    if (m_type == "artist") {
        qCInfo(generic) << "[Music Scraper] Searching for artist:" << input;
        mediaelch::scraper::ArtistSearchJob::Config config;
        config.query = input;
        // config.locale = …
        mediaelch::scraper::ArtistSearchJob* searchJob = scraper->searchArtist(config);
        connect(searchJob,
            &mediaelch::scraper::ArtistSearchJob::searchFinished,
            this,
            &MusicSearchWidget::onArtistSearchFinished);
        searchJob->start();

    } else if (m_type == "album") {
        qCInfo(generic) << "[Music Scraper] Searching for album:" << input << "| artist:" << m_artistName;
        mediaelch::scraper::AlbumSearchJob::Config config;
        config.artistName = m_artistName;
        config.albumQuery = input;
        // config.locale = …
        mediaelch::scraper::AlbumSearchJob* searchJob = scraper->searchAlbum(config);
        connect(searchJob,
            &mediaelch::scraper::AlbumSearchJob::searchFinished,
            this,
            &MusicSearchWidget::onAlbumSearchFinished);
        searchJob->start();
    }
}

void MusicSearchWidget::onAlbumSearchFinished(mediaelch::scraper::AlbumSearchJob* searchJob)
{
    auto dls = makeDeleteLaterScope(searchJob);
    ui->comboScraper->setEnabled(true);
    ui->searchString->setLoading(false);
    ui->searchString->setFocus();

    const auto& results = searchJob->results();
    for (const auto& result : results) {
        auto* label = new MyLabel(ui->results);
        QString name = result.title;
        if (result.released.isValid()) {
            name.append(QStringLiteral(" (%1)").arg(result.released.toString("yyyy")));
        }

        label->setText(name + "<br /><span style=\"color: #999999;\">" + result.identifier + "</span>");
        label->setMargin(8);

        auto* item = new QTableWidgetItem;
        item->setData(Qt::UserRole, result.identifier);
        item->setData(Qt::UserRole + 1, result.groupIdentifier);
        int row = ui->results->rowCount();
        ui->results->insertRow(row);
        ui->results->setItem(row, 0, item);
        ui->results->setCellWidget(row, 0, label);

        connect(label, &MyLabel::clicked, m_signalMapper, elchOverload<>(&QSignalMapper::map));
        m_signalMapper->setMapping(label, row);
    }
}

void MusicSearchWidget::onArtistSearchFinished(mediaelch::scraper::ArtistSearchJob* searchJob)
{
    auto dls = makeDeleteLaterScope(searchJob);
    ui->comboScraper->setEnabled(true);
    ui->searchString->setLoading(false);
    ui->searchString->setFocus();

    const auto& results = searchJob->results();
    for (const auto& result : results) {
        auto* label = new MyLabel(ui->results);
        QString name = result.title;
        if (result.released.isValid()) {
            name.append(QStringLiteral(" (%1)").arg(result.released.toString("yyyy")));
        }

        label->setText(name + "<br /><span style=\"color: #999999;\">" + result.identifier + "</span>");
        label->setMargin(8);

        auto* item = new QTableWidgetItem;
        item->setData(Qt::UserRole, result.identifier);
        int row = ui->results->rowCount();
        ui->results->insertRow(row);
        ui->results->setItem(row, 0, item);
        ui->results->setCellWidget(row, 0, label);

        connect(label, &MyLabel::clicked, m_signalMapper, elchOverload<>(&QSignalMapper::map));
        m_signalMapper->setMapping(label, row);
    }
}

void MusicSearchWidget::resultClicked(QTableWidgetItem* item)
{
    m_scraperId = item->data(Qt::UserRole).toString();
    m_scraperId2 = item->data(Qt::UserRole + 1).toString();
    emit sigResultClicked();
}

void MusicSearchWidget::resultClickedRow(int row)
{
    if (row < 0 || row >= ui->results->rowCount()) {
        return;
    }
    resultClicked(ui->results->item(row, 0));
}

void MusicSearchWidget::chkToggled()
{
    m_infosToLoad.clear();
    bool allToggled = true;
    for (MyCheckBox* box : ui->groupBox->findChildren<MyCheckBox*>()) {
        if (box->isChecked() && box->myData().toInt() > 0 && box->isEnabled() && !box->isHidden()) {
            m_infosToLoad.insert(MusicScraperInfo(box->myData().toInt()));
        }
        if (!box->isChecked() && box->myData().toInt() > 0 && box->isEnabled() && !box->isHidden()) {
            allToggled = false;
        }
    }
    ui->chkUnCheckAll->setChecked(allToggled);

    int scraperNo = ui->comboScraper->itemData(ui->comboScraper->currentIndex(), Qt::UserRole).toInt();
    Settings::instance()->setScraperInfos(m_type + "/" + QString::number(scraperNo), m_infosToLoad);
}

void MusicSearchWidget::chkAllToggled(bool toggled)
{
    for (MyCheckBox* box : ui->groupBox->findChildren<MyCheckBox*>()) {
        if (box->myData().toInt() > 0 && box->isEnabled() && !box->isHidden()) {
            box->setChecked(toggled);
        }
    }
    chkToggled();
}

int MusicSearchWidget::scraperNo() const
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

QSet<MusicScraperInfo> MusicSearchWidget::infosToLoad()
{
    return m_infosToLoad;
}

void MusicSearchWidget::setCheckBoxesEnabled(QSet<MusicScraperInfo> scraperSupports)
{
    int scraperNo = ui->comboScraper->itemData(ui->comboScraper->currentIndex(), Qt::UserRole).toInt();
    QSet<MusicScraperInfo> infos =
        Settings::instance()->scraperInfos<MusicScraperInfo>(m_type + "/" + QString::number(scraperNo));

    for (MyCheckBox* box : ui->groupBox->findChildren<MyCheckBox*>()) {
        box->setEnabled(scraperSupports.contains(MusicScraperInfo(box->myData().toInt())));
        box->setChecked((infos.contains(MusicScraperInfo(box->myData().toInt())) || infos.isEmpty())
                        && scraperSupports.contains(MusicScraperInfo(box->myData().toInt())));
    }
    chkToggled();
}

void MusicSearchWidget::setType(const QString& type)
{
    m_type = type;
    for (MyCheckBox* box : ui->groupBox->findChildren<MyCheckBox*>()) {
        box->setVisible(box->property("type").toString() == "both" || box->property("type").toString() == type);
    }
}

void MusicSearchWidget::setArtistName(const QString& artistName)
{
    m_artistName = artistName;
}
