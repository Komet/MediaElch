#include "TvShowWidgetEpisode.h"
#include "ui_TvShowWidgetEpisode.h"

#include <QMovie>
#include <QPainter>
#include "globals/Globals.h"
#include "globals/ImagePreviewDialog.h"
#include "globals/Manager.h"
#include "main/MessageBox.h"
#include "globals/ImageDialog.h"
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
    ui->thumbnailResolution->clear();
    ui->directors->verticalHeader()->setResizeMode(QHeaderView::ResizeToContents);
    ui->writers->verticalHeader()->setResizeMode(QHeaderView::ResizeToContents);
    ui->buttonPreviewBackdrop->setEnabled(false);

    QFont font = ui->episodeName->font();
    font.setPointSize(font.pointSize()+4);
    ui->episodeName->setFont(font);

    font = ui->thumbnailResolution->font();
    #ifdef Q_WS_WIN
    font.setPointSize(font.pointSize()-1);
    #else
    font.setPointSize(font.pointSize()-2);
    #endif
    ui->thumbnailResolution->setFont(font);

    m_posterDownloadManager = new DownloadManager(this);

    connect(ui->name, SIGNAL(textChanged(QString)), ui->episodeName, SLOT(setText(QString)));
    connect(ui->buttonAddDirector, SIGNAL(clicked()), this, SLOT(onAddDirector()));
    connect(ui->buttonRemoveDirector, SIGNAL(clicked()), this, SLOT(onRemoveDirector()));
    connect(ui->buttonAddWriter, SIGNAL(clicked()), this, SLOT(onAddWriter()));
    connect(ui->buttonRemoveWriter, SIGNAL(clicked()), this, SLOT(onRemoveWriter()));
    connect(ui->thumbnail, SIGNAL(clicked()), this, SLOT(onChooseThumbnail()));
    connect(m_posterDownloadManager, SIGNAL(downloadFinished(DownloadManagerElement)), this, SLOT(onPosterDownloadFinished(DownloadManagerElement)));
    connect(ui->buttonPreviewBackdrop, SIGNAL(clicked()), this, SLOT(onPreviewBackdrop()));
    connect(ui->buttonRevert, SIGNAL(clicked()), this, SLOT(onRevertChanges()));

    onClear();

    // Connect GUI change events to movie object
    connect(ui->name, SIGNAL(textEdited(QString)), this, SLOT(onNameChange(QString)));
    connect(ui->showTitle, SIGNAL(textEdited(QString)), this, SLOT(onShowTitleChange(QString)));
    connect(ui->season, SIGNAL(valueChanged(int)), this, SLOT(onSeasonChange(int)));
    connect(ui->episode, SIGNAL(valueChanged(int)), this, SLOT(onEpisodeChange(int)));
    connect(ui->rating, SIGNAL(valueChanged(double)), this, SLOT(onRatingChange(double)));
    connect(ui->certification, SIGNAL(editTextChanged(QString)), this, SLOT(onCertificationChange(QString)));
    connect(ui->firstAired, SIGNAL(dateChanged(QDate)), this, SLOT(onFirstAiredChange(QDate)));
    connect(ui->playCount, SIGNAL(valueChanged(int)), this, SLOT(onPlayCountChange(int)));
    connect(ui->lastPlayed, SIGNAL(dateTimeChanged(QDateTime)), this, SLOT(onLastPlayedChange(QDateTime)));
    connect(ui->studio, SIGNAL(textEdited(QString)), this, SLOT(onStudioChange(QString)));
    connect(ui->overview, SIGNAL(textChanged()), this, SLOT(onOverviewChange()));

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
    ui->tabWidget->setCurrentIndex(0);
    ui->directors->setRowCount(0);
    ui->writers->setRowCount(0);
    ui->thumbnailResolution->clear();
    ui->thumbnail->setPixmap(QPixmap(":/img/pictures_alt.png").scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    ui->episodeName->clear();
    ui->files->clear();
    ui->files->setToolTip("");
    ui->name->clear();
    ui->showTitle->clear();
    ui->season->clear();
    ui->episode->clear();
    ui->rating->clear();
    ui->firstAired->setDate(QDate::currentDate());
    ui->playCount->clear();
    ui->lastPlayed->setDateTime(QDateTime::currentDateTime());
    ui->studio->clear();
    ui->overview->clear();
    ui->certification->clear();
    ui->buttonRevert->setVisible(false);
}

/**
 * @brief Sets the enabled status of the main group box
 * @param enabled Status
 */
void TvShowWidgetEpisode::onSetEnabled(bool enabled)
{
    qDebug() << "Entered";
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
    updateEpisodeInfo();
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
    ui->rating->blockSignals(true);
    ui->certification->blockSignals(true);
    ui->firstAired->blockSignals(true);
    ui->playCount->blockSignals(true);
    ui->lastPlayed->blockSignals(true);
    ui->overview->blockSignals(true);

    onClear();

    ui->files->setText(m_episode->files().join(", "));
    ui->files->setToolTip(m_episode->files().join("\n"));
    ui->name->setText(m_episode->name());
    ui->showTitle->setText(m_episode->showTitle());
    ui->season->setValue(m_episode->season());
    ui->episode->setValue(m_episode->episode());
    ui->rating->setValue(m_episode->rating());
    ui->firstAired->setDate(m_episode->firstAired());
    ui->playCount->setValue(m_episode->playCount());
    ui->lastPlayed->setDateTime(m_episode->lastPlayed());
    ui->studio->setText(m_episode->network());
    ui->overview->setPlainText(m_episode->overview());

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


    if (!m_episode->thumbnailImage()->isNull()) {
        ui->thumbnail->setPixmap(QPixmap::fromImage(*m_episode->thumbnailImage()).scaledToWidth(200, Qt::SmoothTransformation));
        ui->thumbnailResolution->setText(QString("%1x%2").arg(m_episode->thumbnailImage()->width()).arg(m_episode->thumbnailImage()->height()));
        ui->buttonPreviewBackdrop->setEnabled(true);
        m_currentBackdrop = *m_episode->thumbnailImage();
    } else if (!Manager::instance()->mediaCenterInterface()->thumbnailImageName(m_episode).isEmpty()) {
        QPixmap p(Manager::instance()->mediaCenterInterface()->thumbnailImageName(m_episode));
        ui->thumbnail->setPixmap(p.scaledToWidth(200, Qt::SmoothTransformation));
        ui->thumbnailResolution->setText(QString("%1x%2").arg(p.width()).arg(p.height()));
        ui->buttonPreviewBackdrop->setEnabled(true);
        m_currentBackdrop = p.toImage();
    } else {
        ui->thumbnail->setPixmap(QPixmap(":/img/pictures_alt.png").scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        ui->thumbnailResolution->setText("");
        ui->buttonPreviewBackdrop->setEnabled(false);
    }

    ui->season->blockSignals(false);
    ui->episode->blockSignals(false);
    ui->rating->blockSignals(false);
    ui->certification->blockSignals(false);
    ui->firstAired->blockSignals(false);
    ui->playCount->blockSignals(false);
    ui->lastPlayed->blockSignals(false);
    ui->overview->blockSignals(false);

    ui->certification->setEnabled(Manager::instance()->mediaCenterInterfaceTvShow()->hasFeature(MediaCenterFeatures::EditTvShowEpisodeCertification));
    ui->showTitle->setEnabled(Manager::instance()->mediaCenterInterfaceTvShow()->hasFeature(MediaCenterFeatures::EditTvShowEpisodeShowTitle));
    ui->studio->setEnabled(Manager::instance()->mediaCenterInterfaceTvShow()->hasFeature(MediaCenterFeatures::EditTvShowEpisodeNetwork));
    ui->buttonRevert->setVisible(m_episode->hasChanged());
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
    emit sigSetActionSearchEnabled(false, WidgetTvShows);
    emit sigSetActionSaveEnabled(false, WidgetTvShows);
    TvShowSearch::instance()->setChkUpdateAllVisible(false);
    TvShowSearch::instance()->exec(m_episode->showTitle());
    if (TvShowSearch::instance()->result() == QDialog::Accepted) {
        onSetEnabled(false);
        m_episode->loadData(TvShowSearch::instance()->scraperId(), Manager::instance()->tvScrapers().at(0));
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
        d.imageType = TypeBackdrop;
        d.url = m_episode->thumbnail();
        d.episode = m_episode;
        m_posterDownloadManager->addDownload(d);
        ui->thumbnail->setPixmap(QPixmap());
        ui->thumbnail->setMovie(m_loadingMovie);
        ui->buttonPreviewBackdrop->setEnabled(false);
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

    ImageDialog::instance()->setImageType(TypeBackdrop);
    ImageDialog::instance()->clear();
    QList<Poster> posters;
    if (!m_episode->thumbnail().isEmpty()) {
        Poster p;
        p.originalUrl = m_episode->thumbnail();
        p.thumbUrl = m_episode->thumbnail();
        posters << p;
    }
    ImageDialog::instance()->setDownloads(posters);
    ImageDialog::instance()->exec(ImageDialogType::TvShowThumb);

    if (ImageDialog::instance()->result() == QDialog::Accepted) {
        emit sigSetActionSaveEnabled(false, WidgetTvShows);
        DownloadManagerElement d;
        d.imageType = TypeBackdrop;
        d.url = ImageDialog::instance()->imageUrl();
        d.episode = m_episode;
        m_posterDownloadManager->addDownload(d);
        ui->thumbnail->setPixmap(QPixmap());
        ui->thumbnail->setMovie(m_loadingMovie);
        ui->buttonPreviewBackdrop->setEnabled(false);
        ui->buttonRevert->setVisible(true);
    }
}

/**
 * @brief Adjusts the size of the backdrop to common values (1080p or 720p) and shows the image
 * @param elem Downloaded element
 */
void TvShowWidgetEpisode::onPosterDownloadFinished(DownloadManagerElement elem)
{
    qDebug() << "Entered";
    if (elem.imageType == TypeBackdrop) {
        qDebug() << "Got a backdrop";
        if ((elem.image.width() != 1920 || elem.image.height() != 1080) &&
            elem.image.width() > 1915 && elem.image.width() < 1925 && elem.image.height() > 1075 && elem.image.height() < 1085)
            elem.image = elem.image.scaled(1920, 1080, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

        if ((elem.image.width() != 1280 || elem.image.height() != 720) &&
            elem.image.width() > 1275 && elem.image.width() < 1285 && elem.image.height() > 715 && elem.image.height() < 725)
            elem.image = elem.image.scaled(1280, 720, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

        if (m_episode == elem.episode) {
            ui->thumbnail->setPixmap(QPixmap::fromImage(elem.image).scaled(200, 112, Qt::KeepAspectRatio, Qt::SmoothTransformation));
            ui->thumbnailResolution->setText(QString("%1x%2").arg(elem.image.width()).arg(elem.image.height()));
            ui->buttonPreviewBackdrop->setEnabled(true);
            m_currentBackdrop = elem.image;
        }
        elem.episode->setThumbnailImage(elem.image);
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

/**
 * @brief Shows a full size image of the backdrop
 */
void TvShowWidgetEpisode::onPreviewBackdrop()
{
    ImagePreviewDialog::instance()->setImage(QPixmap::fromImage(m_currentBackdrop));
    ImagePreviewDialog::instance()->exec();
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

/**
 * @brief Marks the episode as changed when the overview has changed
 */
void TvShowWidgetEpisode::onOverviewChange()
{
    m_episode->setOverview(ui->overview->toPlainText());
    ui->buttonRevert->setVisible(true);
}
