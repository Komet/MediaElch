#include "TvShowWidgetEpisode.h"
#include "ui_TvShowWidgetEpisode.h"

#include <QGraphicsProxyWidget>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QMovie>
#include <QPainter>
#include "data/ImageCache.h"
#include "globals/ComboDelegate.h"
#include "globals/Globals.h"
#include "globals/ImageDialog.h"
#include "globals/ImagePreviewDialog.h"
#include "globals/Manager.h"
#include "main/MessageBox.h"
#include "tvShows/TvShowSearch.h"

/**
 * @brief TvShowWidgetEpisode::TvShowWidgetEpisode
 * @param parent
 */
TvShowWidgetEpisode::TvShowWidgetEpisode(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TvShowWidgetEpisode)
{
    ui->setupUi(this);

    m_episode = 0;

    ui->episodeName->clear();
#if QT_VERSION >= 0x050000
    ui->directors->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->writers->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
#else
    ui->directors->verticalHeader()->setResizeMode(QHeaderView::ResizeToContents);
    ui->writers->verticalHeader()->setResizeMode(QHeaderView::ResizeToContents);
#endif

    QFont font = ui->episodeName->font();
    font.setPointSize(font.pointSize()+4);
    ui->episodeName->setFont(font);

    font = ui->labelThumbnail->font();
    #ifdef Q_OS_WIN32
        font.setPointSize(font.pointSize()-1);
    #else
        font.setPointSize(font.pointSize()-2);
    #endif

    font.setBold(true);
    ui->labelThumbnail->setFont(font);

    ui->directors->setItemDelegate(new ComboDelegate(ui->directors, WidgetTvShows, ComboDelegateDirectors));
    ui->writers->setItemDelegate(new ComboDelegate(ui->writers, WidgetTvShows, ComboDelegateWriters));
    ui->thumbnail->setDefaultPixmap(QPixmap(":/img/pictures_alt.png").scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation));

    m_posterDownloadManager = new DownloadManager(this);

    connect(ui->name, SIGNAL(textChanged(QString)), ui->episodeName, SLOT(setText(QString)));
    connect(ui->buttonAddDirector, SIGNAL(clicked()), this, SLOT(onAddDirector()));
    connect(ui->buttonRemoveDirector, SIGNAL(clicked()), this, SLOT(onRemoveDirector()));
    connect(ui->buttonAddWriter, SIGNAL(clicked()), this, SLOT(onAddWriter()));
    connect(ui->buttonRemoveWriter, SIGNAL(clicked()), this, SLOT(onRemoveWriter()));
    connect(ui->thumbnail, SIGNAL(clicked()), this, SLOT(onChooseThumbnail()));
    connect(ui->thumbnail, SIGNAL(sigClose()), this, SLOT(onDeleteThumbnail()));
    connect(m_posterDownloadManager, SIGNAL(downloadFinished(DownloadManagerElement)), this, SLOT(onPosterDownloadFinished(DownloadManagerElement)));
    connect(ui->buttonRevert, SIGNAL(clicked()), this, SLOT(onRevertChanges()));
    connect(ui->buttonReloadStreamDetails, SIGNAL(clicked()), this, SLOT(onReloadStreamDetails()));

    onClear();

    // Connect GUI change events to movie object
    connect(ui->name, SIGNAL(textEdited(QString)), this, SLOT(onNameChange(QString)));
    connect(ui->showTitle, SIGNAL(textEdited(QString)), this, SLOT(onShowTitleChange(QString)));
    connect(ui->season, SIGNAL(valueChanged(int)), this, SLOT(onSeasonChange(int)));
    connect(ui->episode, SIGNAL(valueChanged(int)), this, SLOT(onEpisodeChange(int)));
    connect(ui->displaySeason, SIGNAL(valueChanged(int)), this, SLOT(onDisplaySeasonChange(int)));
    connect(ui->displayEpisode, SIGNAL(valueChanged(int)), this, SLOT(onDisplayEpisodeChange(int)));
    connect(ui->rating, SIGNAL(valueChanged(double)), this, SLOT(onRatingChange(double)));
    connect(ui->certification, SIGNAL(editTextChanged(QString)), this, SLOT(onCertificationChange(QString)));
    connect(ui->firstAired, SIGNAL(dateChanged(QDate)), this, SLOT(onFirstAiredChange(QDate)));
    connect(ui->epBookmark, SIGNAL(timeChanged(QTime)), this, SLOT(onEpBookmarkChange(QTime)));
    connect(ui->playCount, SIGNAL(valueChanged(int)), this, SLOT(onPlayCountChange(int)));
    connect(ui->lastPlayed, SIGNAL(dateTimeChanged(QDateTime)), this, SLOT(onLastPlayedChange(QDateTime)));
    connect(ui->studio, SIGNAL(textEdited(QString)), this, SLOT(onStudioChange(QString)));
    connect(ui->overview, SIGNAL(textChanged()), this, SLOT(onOverviewChange()));
    connect(ui->videoAspectRatio, SIGNAL(valueChanged(double)), this, SLOT(onStreamDetailsEdited()));
    connect(ui->videoCodec, SIGNAL(textEdited(QString)), this, SLOT(onStreamDetailsEdited()));
    connect(ui->videoDuration, SIGNAL(timeChanged(QTime)), this, SLOT(onStreamDetailsEdited()));
    connect(ui->videoHeight, SIGNAL(valueChanged(int)), this, SLOT(onStreamDetailsEdited()));
    connect(ui->videoWidth, SIGNAL(valueChanged(int)), this, SLOT(onStreamDetailsEdited()));
    connect(ui->videoScantype, SIGNAL(textEdited(QString)), this, SLOT(onStreamDetailsEdited()));

    m_loadingMovie = new QMovie(":/img/spinner.gif");
    m_loadingMovie->start();
    m_savingWidget = new QLabel(this);
    m_savingWidget->setMovie(m_loadingMovie);
    m_savingWidget->hide();

    onSetEnabled(false);

    QPainter p;
    QPixmap revert(":/img/arrow_circle_left.png");
    p.begin(&revert);
    p.setCompositionMode(QPainter::CompositionMode_SourceIn);
    p.fillRect(revert.rect(), QColor(0, 0, 0, 200));
    p.end();
    ui->buttonRevert->setIcon(QIcon(revert));
    ui->buttonRevert->setVisible(false);

    QLabel *missingLabel = new QLabel(tr("Episode missing"));
    missingLabel->setStyleSheet("padding-top: 5px; padding-bottom: 5px; color: #f0f0f0; font-size: 18px; "
                       "background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:0, y2:1, stop:0 rgba(238, 95, 91, 255), stop:1 rgba(189, 53, 47, 255));"
                       "border-top: 1px solid rgba(255, 255, 255, 80); border-bottom: 1px solid rgba(255, 255, 255, 80)");
    missingLabel->setFixedWidth(300);
    missingLabel->setAlignment(Qt::AlignCenter);
    QGraphicsScene *scene = new QGraphicsScene(this);
    QGraphicsProxyWidget *proxy = scene->addWidget(missingLabel);
    proxy->rotate(-45);
    proxy->setMaximumHeight(300);
    proxy->setMaximumWidth(300);
    m_missingView = new QGraphicsView(scene);
    m_missingView->setFixedSize(300, 300);
    m_missingView->move(-65, -65);
    m_missingView->setStyleSheet("background-color: transparent; border: none;");
    m_missingView->setParent(ui->groupBox_3);
    m_missingView->setVisible(false);
}

/**
 * @brief TvShowWidgetEpisode::~TvShowWidgetEpisode
 */
TvShowWidgetEpisode::~TvShowWidgetEpisode()
{
    delete ui;
}

/**
 * @brief Repositions the saving widget
 * @param event
 */
void TvShowWidgetEpisode::resizeEvent(QResizeEvent *event)
{
    m_savingWidget->move(size().width()/2-m_savingWidget->width(), height()/2-m_savingWidget->height());
    QWidget::resizeEvent(event);
}

/**
 * @brief Clears all contents
 */
void TvShowWidgetEpisode::onClear()
{
    bool blocked = false;

    ui->directors->setRowCount(0);
    ui->writers->setRowCount(0);
    ui->thumbnail->clear();

    blocked = ui->episodeName->blockSignals(true);
    ui->episodeName->clear();
    ui->episodeName->blockSignals(blocked);

    ui->files->clear();
    ui->files->setToolTip("");

    blocked = ui->name->blockSignals(true);
    ui->name->clear();
    ui->name->blockSignals(blocked);

    blocked = ui->showTitle->blockSignals(true);
    ui->showTitle->clear();
    ui->showTitle->blockSignals(blocked);

    blocked = ui->season->blockSignals(true);
    ui->season->clear();
    ui->season->blockSignals(blocked);

    blocked = ui->episode->blockSignals(true);
    ui->episode->clear();
    ui->episode->blockSignals(blocked);

    blocked = ui->displaySeason->blockSignals(true);
    ui->displaySeason->clear();
    ui->displaySeason->blockSignals(blocked);

    blocked = ui->displayEpisode->blockSignals(true);
    ui->displayEpisode->clear();
    ui->displayEpisode->blockSignals(blocked);

    blocked = ui->rating->blockSignals(true);
    ui->rating->clear();
    ui->rating->blockSignals(blocked);

    blocked = ui->firstAired->blockSignals(true);
    ui->firstAired->setDate(QDate::currentDate());
    ui->firstAired->blockSignals(blocked);

    blocked = ui->playCount->blockSignals(true);
    ui->playCount->clear();
    ui->playCount->blockSignals(blocked);

    blocked = ui->lastPlayed->blockSignals(true);
    ui->lastPlayed->setDateTime(QDateTime::currentDateTime());
    ui->lastPlayed->blockSignals(blocked);

    blocked = ui->studio->blockSignals(true);
    ui->studio->clear();
    ui->studio->blockSignals(blocked);

    blocked = ui->overview->blockSignals(true);
    ui->overview->clear();
    ui->overview->blockSignals(blocked);

    blocked = ui->certification->blockSignals(true);
    ui->certification->clear();
    ui->certification->blockSignals(blocked);

    blocked = ui->videoCodec->blockSignals(true);
    ui->videoCodec->clear();
    ui->videoCodec->blockSignals(blocked);

    blocked = ui->videoScantype->blockSignals(true);
    ui->videoScantype->clear();
    ui->videoScantype->blockSignals(blocked);

    blocked = ui->videoAspectRatio->blockSignals(true);
    ui->videoAspectRatio->clear();
    ui->videoAspectRatio->blockSignals(blocked);

    blocked = ui->videoDuration->blockSignals(true);
    ui->videoDuration->clear();
    ui->videoDuration->blockSignals(blocked);

    blocked = ui->videoHeight->blockSignals(true);
    ui->videoHeight->clear();
    ui->videoHeight->blockSignals(blocked);

    blocked = ui->videoWidth->blockSignals(true);
    ui->videoWidth->clear();
    ui->videoWidth->blockSignals(blocked);

    blocked = ui->epBookmark->blockSignals(true);
    ui->epBookmark->setTime(QTime(0, 0, 0));
    ui->epBookmark->blockSignals(blocked);

    ui->buttonRevert->setVisible(false);
    ui->mediaFlags->clear();
}

/**
 * @brief Sets the enabled status of the main group box
 * @param enabled Status
 */
void TvShowWidgetEpisode::onSetEnabled(bool enabled)
{
    qDebug() << "Entered";
    if (m_episode && m_episode->isDummy()) {
        ui->groupBox_3->setEnabled(false);
        return;
    }
    ui->groupBox_3->setEnabled(enabled);
}

/**
 * @brief Sets the episode and tells the episode to load its images
 * @param episode Episode to set
 */
void TvShowWidgetEpisode::setEpisode(TvShowEpisode *episode)
{
    qDebug() << "Entered, episode=" << episode->name();
    m_episode = episode;
    if (!episode->streamDetailsLoaded() && Settings::instance()->autoLoadStreamDetails() && !episode->isDummy())
        episode->loadStreamDetailsFromFile();
    m_missingView->setVisible(episode->isDummy());
    updateEpisodeInfo();

    emit sigSetActionSearchEnabled(!episode->isDummy(), WidgetTvShows);
    emit sigSetActionSaveEnabled(!episode->isDummy(), WidgetTvShows);
}

/**
 * @brief Updates the widgets contents with the episode info
 */
void TvShowWidgetEpisode::updateEpisodeInfo()
{
    qDebug() << "Entered";
    if (m_episode == 0) {
        qWarning() << "My episode is invalid";
        return;
    }

    ui->season->blockSignals(true);
    ui->episode->blockSignals(true);
    ui->displaySeason->blockSignals(true);
    ui->displayEpisode->blockSignals(true);
    ui->rating->blockSignals(true);
    ui->certification->blockSignals(true);
    ui->firstAired->blockSignals(true);
    ui->playCount->blockSignals(true);
    ui->lastPlayed->blockSignals(true);
    ui->overview->blockSignals(true);
    ui->epBookmark->blockSignals(true);

    onClear();

    ui->files->setText(m_episode->files().join(", "));
    ui->files->setToolTip(m_episode->files().join("\n"));
    ui->name->setText(m_episode->name());
    ui->showTitle->setText(m_episode->showTitle());
    ui->season->setValue(m_episode->season());
    ui->episode->setValue(m_episode->episode());
    ui->displaySeason->setValue(m_episode->displaySeason());
    ui->displayEpisode->setValue(m_episode->displayEpisode());
    ui->rating->setValue(m_episode->rating());
    ui->firstAired->setDate(m_episode->firstAired());
    ui->playCount->setValue(m_episode->playCount());
    ui->lastPlayed->setDateTime(m_episode->lastPlayed());
    ui->studio->setText(m_episode->network());
    ui->overview->setPlainText(m_episode->overview());
    ui->epBookmark->setTime(m_episode->epBookmark());

    ui->writers->blockSignals(true);
    foreach (QString *writer, m_episode->writersPointer()) {
        int row = ui->writers->rowCount();
        ui->writers->insertRow(row);
        ui->writers->setItem(row, 0, new QTableWidgetItem(*writer));
        ui->writers->item(row, 0)->setData(Qt::UserRole, QVariant::fromValue(writer));
    }
    ui->writers->blockSignals(false);

    ui->directors->blockSignals(true);
    foreach (QString *director, m_episode->directorsPointer()) {
        int row = ui->directors->rowCount();
        ui->directors->insertRow(row);
        ui->directors->setItem(row, 0, new QTableWidgetItem(*director));
        ui->directors->item(row, 0)->setData(Qt::UserRole, QVariant::fromValue(director));
    }
    ui->writers->blockSignals(false);

    if (m_episode->tvShow()) {
        QStringList certifications = m_episode->tvShow()->certifications();
        certifications.prepend("");
        ui->certification->addItems(certifications);
        ui->certification->setCurrentIndex(certifications.indexOf(m_episode->certification()));
    } else {
        ui->certification->addItem(m_episode->certification());
    }

    // Streamdetails
    updateStreamDetails();
    ui->videoAspectRatio->setEnabled(m_episode->streamDetailsLoaded());
    ui->videoCodec->setEnabled(m_episode->streamDetailsLoaded());
    ui->videoDuration->setEnabled(m_episode->streamDetailsLoaded());
    ui->videoHeight->setEnabled(m_episode->streamDetailsLoaded());
    ui->videoWidth->setEnabled(m_episode->streamDetailsLoaded());
    ui->videoScantype->setEnabled(m_episode->streamDetailsLoaded());

    if (!m_episode->thumbnailImage().isNull())
        ui->thumbnail->setImage(m_episode->thumbnailImage());
    else if (!Manager::instance()->mediaCenterInterface()->imageFileName(m_episode, ImageType::TvShowEpisodeThumb).isEmpty())
        ui->thumbnail->setImage(Manager::instance()->mediaCenterInterface()->imageFileName(m_episode, ImageType::TvShowEpisodeThumb));

    ui->season->blockSignals(false);
    ui->episode->blockSignals(false);
    ui->displaySeason->blockSignals(false);
    ui->displayEpisode->blockSignals(false);
    ui->rating->blockSignals(false);
    ui->certification->blockSignals(false);
    ui->firstAired->blockSignals(false);
    ui->playCount->blockSignals(false);
    ui->lastPlayed->blockSignals(false);
    ui->overview->blockSignals(false);
    ui->epBookmark->blockSignals(false);

    ui->certification->setEnabled(Manager::instance()->mediaCenterInterfaceTvShow()->hasFeature(MediaCenterFeatures::EditTvShowEpisodeCertification));
    ui->showTitle->setEnabled(Manager::instance()->mediaCenterInterfaceTvShow()->hasFeature(MediaCenterFeatures::EditTvShowEpisodeShowTitle));
    ui->studio->setEnabled(Manager::instance()->mediaCenterInterfaceTvShow()->hasFeature(MediaCenterFeatures::EditTvShowEpisodeNetwork));
    ui->buttonRevert->setVisible(m_episode->hasChanged());
}

/**
 * @brief Fills the widget with streamdetails
 * @param reloadFromFile If true forces a reload of streamdetails from the file
 */
void TvShowWidgetEpisode::updateStreamDetails(bool reloadFromFile)
{
    if (m_episode && m_episode->isDummy())
        return;

    ui->videoAspectRatio->blockSignals(true);
    ui->videoDuration->blockSignals(true);
    ui->videoWidth->blockSignals(true);
    ui->videoHeight->blockSignals(true);

    if (reloadFromFile)
        m_episode->loadStreamDetailsFromFile();

    StreamDetails *streamDetails = m_episode->streamDetails();
    ui->videoWidth->setValue(streamDetails->videoDetails().value("width").toInt());
    ui->videoHeight->setValue(streamDetails->videoDetails().value("height").toInt());
    ui->videoAspectRatio->setValue(streamDetails->videoDetails().value("aspect").toDouble());
    ui->videoCodec->setText(streamDetails->videoDetails().value("codec"));
    ui->videoScantype->setText(streamDetails->videoDetails().value("scantype"));
    QTime time;
    time = time.addSecs(streamDetails->videoDetails().value("durationinseconds").toInt());
    ui->videoDuration->setTime(time);

    foreach (QWidget *widget, m_streamDetailsWidgets)
        widget->deleteLater();
    m_streamDetailsWidgets.clear();
    m_streamDetailsAudio.clear();
    m_streamDetailsSubtitles.clear();

    int audioTracks = streamDetails->audioDetails().count();
    for (int i=0 ; i<audioTracks ; ++i) {
        QLabel *label = new QLabel(tr("Track %1").arg(i+1));
        ui->streamDetails->addWidget(label, 7+i, 0);
        QLineEdit *edit1 = new QLineEdit(streamDetails->audioDetails().at(i).value("language"));
        QLineEdit *edit2 = new QLineEdit(streamDetails->audioDetails().at(i).value("codec"));
        QLineEdit *edit3 = new QLineEdit(streamDetails->audioDetails().at(i).value("channels"));
        edit3->setMaximumWidth(50);
        edit1->setToolTip(tr("Language"));
        edit2->setToolTip(tr("Codec"));
        edit3->setToolTip(tr("Channels"));
        edit1->setPlaceholderText(tr("Language"));
        edit2->setPlaceholderText(tr("Codec"));
        edit2->setPlaceholderText(tr("Channels"));
        QHBoxLayout *layout = new QHBoxLayout();
        layout->addWidget(edit1);
        layout->addWidget(edit2);
        layout->addWidget(edit3);
        layout->addStretch(10);
        ui->streamDetails->addLayout(layout, 7+i, 1);
        m_streamDetailsWidgets << label << edit1 << edit2 << edit3;
        m_streamDetailsAudio << (QList<QLineEdit*>() << edit1 << edit2 << edit3);
        connect(edit1, SIGNAL(textEdited(QString)), this, SLOT(onStreamDetailsEdited()));
        connect(edit2, SIGNAL(textEdited(QString)), this, SLOT(onStreamDetailsEdited()));
        connect(edit3, SIGNAL(textEdited(QString)), this, SLOT(onStreamDetailsEdited()));
    }

    if (!streamDetails->subtitleDetails().isEmpty()) {
        QLabel *label = new QLabel(tr("Subtitles"));
        QFont font = label->font();
        font.setBold(true);
        label->setFont(font);
        ui->streamDetails->addWidget(label, 7+audioTracks, 0);
        m_streamDetailsWidgets << label;

        for (int i=0, n=streamDetails->subtitleDetails().count() ; i<n ; ++i) {
            QLabel *label = new QLabel(tr("Track %1").arg(i+1));
            ui->streamDetails->addWidget(label, 8+audioTracks+i, 0);
            QLineEdit *edit1 = new QLineEdit(streamDetails->subtitleDetails().at(i).value("language"));
            edit1->setToolTip(tr("Language"));
            edit1->setPlaceholderText(tr("Language"));
            QHBoxLayout *layout = new QHBoxLayout();
            layout->addWidget(edit1);
            layout->addStretch(10);
            ui->streamDetails->addLayout(layout, 8+audioTracks+i, 1);
            m_streamDetailsWidgets << label << edit1;
            m_streamDetailsSubtitles << (QList<QLineEdit*>() << edit1);
            connect(edit1, SIGNAL(textEdited(QString)), this, SLOT(onStreamDetailsEdited()));
        }
    }

    // Media Flags
    ui->mediaFlags->setStreamDetails(streamDetails);

    ui->videoAspectRatio->blockSignals(false);
    ui->videoDuration->blockSignals(false);
    ui->videoWidth->blockSignals(false);
    ui->videoHeight->blockSignals(false);
}

/**
 * @brief Forces a reload of stream details
 */
void TvShowWidgetEpisode::onReloadStreamDetails()
{
    updateStreamDetails(true);
    ui->videoAspectRatio->setEnabled(true);
    ui->videoCodec->setEnabled(true);
    ui->videoDuration->setEnabled(true);
    ui->videoHeight->setEnabled(true);
    ui->videoWidth->setEnabled(true);
    ui->videoScantype->setEnabled(true);
}

/**
 * @brief Saves episodes infos
 */
void TvShowWidgetEpisode::onSaveInformation()
{
    qDebug() << "Entered";
    if (m_episode == 0) {
        qWarning() << "My episode is invalid";
        return;
    }

    if (m_episode->isDummy())
        return;

    onSetEnabled(false);
    m_savingWidget->show();
    m_episode->saveData(Manager::instance()->mediaCenterInterfaceTvShow());
    onSetEnabled(true);
    m_savingWidget->hide();
    ui->buttonRevert->setVisible(false);
    MessageBox::instance()->showMessage(tr("Episode Saved"));
}

/**
 * @brief Reverts changes
 */
void TvShowWidgetEpisode::onRevertChanges()
{
    qDebug() << "Entered";
    m_episode->loadData(Manager::instance()->mediaCenterInterfaceTvShow());
    updateEpisodeInfo();
}

/**
 * @brief Shows the search widget
 */
void TvShowWidgetEpisode::onStartScraperSearch()
{
    qDebug() << "Entered";
    if (m_episode == 0) {
        qWarning() << "My episode is invalid";
        return;
    }

    if (m_episode->isDummy())
        return;

    emit sigSetActionSearchEnabled(false, WidgetTvShows);
    emit sigSetActionSaveEnabled(false, WidgetTvShows);
    TvShowSearch::instance()->setSearchType(TypeEpisode);
    TvShowSearch::instance()->exec(m_episode->showTitle());
    if (TvShowSearch::instance()->result() == QDialog::Accepted) {
        onSetEnabled(false);
        m_episode->loadData(TvShowSearch::instance()->scraperId(), Manager::instance()->tvScrapers().at(0), TvShowSearch::instance()->infosToLoad());
        connect(m_episode, SIGNAL(sigLoaded()), this, SLOT(onLoadDone()), Qt::UniqueConnection);
    } else {
        emit sigSetActionSearchEnabled(true, WidgetTvShows);
        emit sigSetActionSaveEnabled(true, WidgetTvShows);
    }
}

/**
 * @brief Called when the search widget finishes
 * Updates infos and starts downloads
 */
void TvShowWidgetEpisode::onLoadDone()
{
    qDebug() << "Entered";
    if (m_episode == 0) {
        qWarning() << "My episode is invalid";
        return;
    }

    updateEpisodeInfo();
    onSetEnabled(true);

    if (!m_episode->thumbnail().isEmpty()) {
        DownloadManagerElement d;
        d.imageType = ImageType::TvShowEpisodeThumb;
        d.url = m_episode->thumbnail();
        d.episode = m_episode;
        d.directDownload = true;
        m_posterDownloadManager->addDownload(d);
        ui->thumbnail->setLoading(true);
    } else {
        emit sigSetActionSearchEnabled(true, WidgetTvShows);
        emit sigSetActionSaveEnabled(true, WidgetTvShows);
    }
    ui->buttonRevert->setVisible(true);
}

/**
 * @brief Shows the MovieImageDialog and after successful execution starts downloads
 */
void TvShowWidgetEpisode::onChooseThumbnail()
{
    qDebug() << "Entered";

    if (m_episode == 0) {
        qWarning() << "My episode is invalid";
        return;
    }

    ImageDialog::instance()->setImageType(ImageType::TvShowEpisodeThumb);
    ImageDialog::instance()->clear();
    ImageDialog::instance()->setTvShowEpisode(m_episode);
    QList<Poster> posters;
    if (!m_episode->thumbnail().isEmpty()) {
        Poster p;
        p.originalUrl = m_episode->thumbnail();
        p.thumbUrl = m_episode->thumbnail();
        posters << p;
    }
    ImageDialog::instance()->setDownloads(posters);
    ImageDialog::instance()->exec(ImageType::TvShowEpisodeThumb);

    if (ImageDialog::instance()->result() == QDialog::Accepted) {
        emit sigSetActionSaveEnabled(false, WidgetTvShows);
        DownloadManagerElement d;
        d.imageType = ImageType::TvShowEpisodeThumb;
        d.url = ImageDialog::instance()->imageUrl();
        d.episode = m_episode;
        d.directDownload = true;
        m_posterDownloadManager->addDownload(d);
        ui->thumbnail->setLoading(true);
        ui->buttonRevert->setVisible(true);
    }
}

/**
 * @brief Adjusts the size of the backdrop to common values (1080p or 720p) and shows the image
 * @param elem Downloaded element
 */
void TvShowWidgetEpisode::onPosterDownloadFinished(DownloadManagerElement elem)
{
    if (elem.imageType == ImageType::TvShowEpisodeThumb) {
        qDebug() << "Got a backdrop";
        if (m_episode == elem.episode)
            ui->thumbnail->setImage(elem.data);
        ImageCache::instance()->invalidateImages(Manager::instance()->mediaCenterInterface()->imageFileName(elem.episode, ImageType::TvShowEpisodeThumb));
        elem.episode->setThumbnailImage(elem.data);
    }
    if (m_posterDownloadManager->downloadQueueSize() == 0) {
        emit sigSetActionSaveEnabled(true, WidgetTvShows);
        emit sigSetActionSearchEnabled(true, WidgetTvShows);
    }
}

/*** add/remove/edit Actors, Genres, Countries and Studios ***/

/**
 * @brief Adds a director
 */
void TvShowWidgetEpisode::onAddDirector()
{
    QString d = tr("Unknown Director");
    m_episode->addDirector(d);
    QString *director = m_episode->directorsPointer().last();

    ui->directors->blockSignals(true);
    int row = ui->directors->rowCount();
    ui->directors->insertRow(row);
    ui->directors->setItem(row, 0, new QTableWidgetItem(d));
    ui->directors->item(row, 0)->setData(Qt::UserRole, QVariant::fromValue(director));
    ui->directors->scrollToBottom();
    ui->directors->blockSignals(false);
    ui->buttonRevert->setVisible(true);
}

/**
 * @brief Removes a director
 */
void TvShowWidgetEpisode::onRemoveDirector()
{
    int row = ui->directors->currentRow();
    if (row < 0 || row >= ui->directors->rowCount() || !ui->directors->currentItem()->isSelected())
        return;

    QString *director = ui->directors->item(row, 0)->data(Qt::UserRole).value<QString*>();
    m_episode->removeDirector(director);
    ui->directors->blockSignals(true);
    ui->directors->removeRow(row);
    ui->directors->blockSignals(false);
    ui->buttonRevert->setVisible(true);
}

/**
 * @brief Stores the changed values
 * @param item Edited item
 */
void TvShowWidgetEpisode::onDirectorEdited(QTableWidgetItem *item)
{
    QString *director = ui->directors->item(item->row(), 0)->data(Qt::UserRole).value<QString*>();
    director->clear();
    director->append(item->text());
    m_episode->setChanged(true);
    ui->buttonRevert->setVisible(true);
}

/**
 * @brief Adds a writer
 */
void TvShowWidgetEpisode::onAddWriter()
{
    QString w = tr("Unknown Writer");
    m_episode->addWriter(w);
    QString *writer = m_episode->writersPointer().last();

    ui->writers->blockSignals(true);
    int row = ui->writers->rowCount();
    ui->writers->insertRow(row);
    ui->writers->setItem(row, 0, new QTableWidgetItem(w));
    ui->writers->item(row, 0)->setData(Qt::UserRole, QVariant::fromValue(writer));
    ui->writers->scrollToBottom();
    ui->writers->blockSignals(false);
    ui->buttonRevert->setVisible(true);
}

/**
 * @brief Removes a writer
 */
void TvShowWidgetEpisode::onRemoveWriter()
{
    int row = ui->writers->currentRow();
    if (row < 0 || row >= ui->writers->rowCount() || !ui->writers->currentItem()->isSelected())
        return;

    QString *writer = ui->writers->item(row, 0)->data(Qt::UserRole).value<QString*>();
    m_episode->removeWriter(writer);
    ui->writers->blockSignals(true);
    ui->writers->removeRow(row);
    ui->writers->blockSignals(false);
    ui->buttonRevert->setVisible(true);
}

/**
 * @brief Stores the changed values
 * @param item Edited item
 */
void TvShowWidgetEpisode::onWriterEdited(QTableWidgetItem *item)
{
    QString *writer = ui->writers->item(item->row(), 0)->data(Qt::UserRole).value<QString*>();
    writer->clear();
    writer->append(item->text());
    m_episode->setChanged(true);
    ui->buttonRevert->setVisible(true);
}

/*** Pass GUI events to episode object ***/

/**
 * @brief Marks the episode as changed when the name has changed
 */
void TvShowWidgetEpisode::onNameChange(QString text)
{
    m_episode->setName(text);
    ui->buttonRevert->setVisible(true);
}

/**
 * @brief Marks the episode as changed when the show title has changed
 */
void TvShowWidgetEpisode::onShowTitleChange(QString text)
{
    m_episode->setShowTitle(text);
    ui->buttonRevert->setVisible(true);
}

/**
 * @brief Marks the episode as changed when the season has changed
 */
void TvShowWidgetEpisode::onSeasonChange(int value)
{
    m_episode->setSeason(value);
    ui->buttonRevert->setVisible(true);
}

/**
 * @brief Marks the episode as changed when the episode has changed
 */
void TvShowWidgetEpisode::onEpisodeChange(int value)
{
    m_episode->setEpisode(value);
    ui->buttonRevert->setVisible(true);
}

/**
 * @brief Marks the episode as changed when the display season has changed
 */
void TvShowWidgetEpisode::onDisplaySeasonChange(int value)
{
    m_episode->setDisplaySeason(value);
    ui->buttonRevert->setVisible(true);
}

/**
 * @brief Marks the episode as changed when the display episode has changed
 */
void TvShowWidgetEpisode::onDisplayEpisodeChange(int value)
{
    m_episode->setDisplayEpisode(value);
    ui->buttonRevert->setVisible(true);
}

/**
 * @brief Marks the episode as changed when the rating has changed
 */
void TvShowWidgetEpisode::onRatingChange(double value)
{
    m_episode->setRating(value);
    ui->buttonRevert->setVisible(true);
}

/**
 * @brief Marks the episode as changed when the overview has changed
 */
void TvShowWidgetEpisode::onCertificationChange(QString text)
{
    m_episode->setCertification(text);
    ui->buttonRevert->setVisible(true);
}

/**
 * @brief Marks the episode as changed when the first aired date has changed
 */
void TvShowWidgetEpisode::onFirstAiredChange(QDate date)
{
    m_episode->setFirstAired(date);
    ui->buttonRevert->setVisible(true);
}

/**
 * @brief Marks the episode as changed when the playcount has changed
 */
void TvShowWidgetEpisode::onPlayCountChange(int value)
{
    m_episode->setPlayCount(value);
    ui->buttonRevert->setVisible(true);
}

/**
 * @brief Marks the episode as changed when the last played date has changed
 */
void TvShowWidgetEpisode::onLastPlayedChange(QDateTime dateTime)
{
    m_episode->setLastPlayed(dateTime);
    ui->buttonRevert->setVisible(true);
}

/**
 * @brief Marks the episode as changed when the studio has changed
 */
void TvShowWidgetEpisode::onStudioChange(QString text)
{
    m_episode->setNetwork(text);
    ui->buttonRevert->setVisible(true);
}

void TvShowWidgetEpisode::onEpBookmarkChange(QTime time)
{
    if (!m_episode)
        return;
    m_episode->setEpBookmark(time);
    ui->buttonRevert->setVisible(true);
}

/**
 * @brief Marks the episode as changed when the overview has changed
 */
void TvShowWidgetEpisode::onOverviewChange()
{
    m_episode->setOverview(ui->overview->toPlainText());
    ui->buttonRevert->setVisible(true);
}

void TvShowWidgetEpisode::onDeleteThumbnail()
{
    m_episode->removeImage(ImageType::TvShowEpisodeThumb);
    if (!m_episode->imagesToRemove().contains(ImageType::TvShowEpisodeThumb) && !Manager::instance()->mediaCenterInterface()->imageFileName(m_episode, ImageType::TvShowEpisodeThumb).isEmpty())
        ui->thumbnail->setImage(Manager::instance()->mediaCenterInterface()->imageFileName(m_episode, ImageType::TvShowEpisodeThumb));
    ui->buttonRevert->setVisible(true);
}

/**
 * @brief Updates all stream details for this episode with values from the widget
 */
void TvShowWidgetEpisode::onStreamDetailsEdited()
{
    StreamDetails *details = m_episode->streamDetails();
    details->setVideoDetail("codec", ui->videoCodec->text());
    details->setVideoDetail("aspect", ui->videoAspectRatio->text());
    details->setVideoDetail("width", ui->videoWidth->text());
    details->setVideoDetail("height", ui->videoHeight->text());
    details->setVideoDetail("scantype", ui->videoScantype->text());
    details->setVideoDetail("durationinseconds", QString("%1").arg(-ui->videoDuration->time().secsTo(QTime(0, 0))));

    for (int i=0, n=m_streamDetailsAudio.count() ; i<n ; ++i) {
        details->setAudioDetail(i, "language", m_streamDetailsAudio[i][0]->text());
        details->setAudioDetail(i, "codec", m_streamDetailsAudio[i][1]->text());
        details->setAudioDetail(i, "channels", m_streamDetailsAudio[i][2]->text());
    }
    for (int i=0, n=m_streamDetailsSubtitles.count() ; i<n ; ++i)
        details->setSubtitleDetail(i, "language", m_streamDetailsSubtitles[i][0]->text());

    m_episode->setChanged(true);
    ui->buttonRevert->setVisible(true);
}

