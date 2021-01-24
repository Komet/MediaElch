#include "TvShowWidgetEpisode.h"
#include "ui_TvShowWidgetEpisode.h"

#include "data/ImageCache.h"
#include "globals/ComboDelegate.h"
#include "globals/Globals.h"
#include "globals/Helper.h"
#include "globals/ImageDialog.h"
#include "globals/ImagePreviewDialog.h"
#include "globals/Manager.h"
#include "globals/MessageIds.h"
#include "image/ImageCapture.h"
#include "ui/notifications/NotificationBox.h"
#include "ui/tv_show/TvShowSearch.h"

#include <QBuffer>
#include <QFileDialog>
#include <QHeaderView>
#include <QMovie>
#include <QPainter>

TvShowWidgetEpisode::TvShowWidgetEpisode(QWidget* parent) :
    QWidget(parent), ui(new Ui::TvShowWidgetEpisode), m_episode{nullptr}
{
    ui->setupUi(this);

    ui->episodeName->clear();
    ui->directors->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->writers->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->actors->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->actors->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

#ifndef Q_OS_MAC
    QFont nameFont = ui->episodeName->font();
    nameFont.setPointSize(nameFont.pointSize() - 4);
    ui->episodeName->setFont(nameFont);
#endif

    QFont font = ui->labelThumbnail->font();
#ifdef Q_OS_WIN
    font.setPointSize(font.pointSize() - 1);
#else
    font.setPointSize(font.pointSize() - 2);
#endif
    ui->labelThumbnail->setFont(font);

    font = ui->actorResolution->font();
#ifdef Q_OS_WIN
    font.setPointSize(font.pointSize() - 1);
#else
    font.setPointSize(font.pointSize() - 2);
#endif
    ui->actorResolution->setFont(font);

    ui->directors->setItemDelegate(
        new ComboDelegate(ui->directors, MainWidgets::TvShows, ComboDelegateType::Directors));
    ui->writers->setItemDelegate(new ComboDelegate(ui->writers, MainWidgets::TvShows, ComboDelegateType::Writers));
    ui->thumbnail->setDefaultPixmap(QPixmap(":/img/placeholders/thumb.png"));
    ui->thumbnail->setShowCapture(true);

    m_posterDownloadManager = new DownloadManager(this);

    connect(ui->name, &QLineEdit::textChanged, ui->episodeName, &QLabel::setText);
    connect(ui->buttonAddDirector, &QAbstractButton::clicked, this, &TvShowWidgetEpisode::onAddDirector);
    connect(ui->buttonRemoveDirector, &QAbstractButton::clicked, this, &TvShowWidgetEpisode::onRemoveDirector);
    connect(ui->buttonAddWriter, &QAbstractButton::clicked, this, &TvShowWidgetEpisode::onAddWriter);
    connect(ui->buttonRemoveWriter, &QAbstractButton::clicked, this, &TvShowWidgetEpisode::onRemoveWriter);
    connect(m_posterDownloadManager,
        &DownloadManager::sigDownloadFinished,
        this,
        &TvShowWidgetEpisode::onPosterDownloadFinished,
        static_cast<Qt::ConnectionType>(Qt::QueuedConnection | Qt::UniqueConnection));
    connect(ui->buttonRevert, &QAbstractButton::clicked, this, &TvShowWidgetEpisode::onRevertChanges);
    connect(
        ui->buttonReloadStreamDetails, &QAbstractButton::clicked, this, &TvShowWidgetEpisode::onReloadStreamDetails);
    connect(ui->buttonAddActor, &QAbstractButton::clicked, this, &TvShowWidgetEpisode::onAddActor);
    connect(ui->buttonRemoveActor, &QAbstractButton::clicked, this, &TvShowWidgetEpisode::onRemoveActor);
    connect(ui->actors, &QTableWidget::itemSelectionChanged, this, &TvShowWidgetEpisode::onActorChanged);
    connect(ui->actor, &MyLabel::clicked, this, &TvShowWidgetEpisode::onChangeActorImage);
    connect(ui->thumbnail, &ClosableImage::clicked, this, &TvShowWidgetEpisode::onChooseThumbnail);
    connect(ui->thumbnail, &ClosableImage::sigClose, this, &TvShowWidgetEpisode::onDeleteThumbnail);
    connect(ui->thumbnail, &ClosableImage::sigImageDropped, this, &TvShowWidgetEpisode::onImageDropped);
    connect(ui->thumbnail, &ClosableImage::sigCapture, this, &TvShowWidgetEpisode::onCaptureImage);

    // TODO: TagCloud: Remove label per default
    ui->tagCloud->hideLabel();
    ui->tagCloud->setPlaceholder(tr("Add Tag"));
    connect(ui->tagCloud, &TagCloud::activated, this, &TvShowWidgetEpisode::onAddTag);
    connect(ui->tagCloud, &TagCloud::deactivated, this, &TvShowWidgetEpisode::onRemoveTag);

    onClear();

    // Connect GUI change events to TV show object
    connect(ui->imdbId, &QLineEdit::textEdited, this, &TvShowWidgetEpisode::onImdbIdChanged);
    connect(ui->tvdbId, &QLineEdit::textEdited, this, &TvShowWidgetEpisode::onTvdbIdChanged);
    connect(ui->tvmazeId, &QLineEdit::textEdited, this, &TvShowWidgetEpisode::onTvmazeIdChanged);
    connect(ui->name, &QLineEdit::textEdited, this, &TvShowWidgetEpisode::onNameChange);
    connect(ui->showTitle, &QLineEdit::textEdited, this, &TvShowWidgetEpisode::onShowTitleChange);
    connect(ui->season, elchOverload<int>(&QSpinBox::valueChanged), this, &TvShowWidgetEpisode::onSeasonChange);
    connect(ui->episode, elchOverload<int>(&QSpinBox::valueChanged), this, &TvShowWidgetEpisode::onEpisodeChange);
    connect(ui->displaySeason,
        elchOverload<int>(&QSpinBox::valueChanged),
        this,
        &TvShowWidgetEpisode::onDisplaySeasonChange);
    connect(ui->displayEpisode,
        elchOverload<int>(&QSpinBox::valueChanged),
        this,
        &TvShowWidgetEpisode::onDisplayEpisodeChange);
    connect(
        ui->rating, elchOverload<double>(&QDoubleSpinBox::valueChanged), this, &TvShowWidgetEpisode::onRatingChange);
    connect(ui->votes, elchOverload<int>(&QSpinBox::valueChanged), this, &TvShowWidgetEpisode::onVotesChange);
    connect(ui->top250, elchOverload<int>(&QSpinBox::valueChanged), this, &TvShowWidgetEpisode::onTop250Change);
    connect(ui->certification, &QComboBox::editTextChanged, this, &TvShowWidgetEpisode::onCertificationChange);
    connect(ui->firstAired, &QDateTimeEdit::dateChanged, this, &TvShowWidgetEpisode::onFirstAiredChange);
    connect(ui->epBookmark, &QDateTimeEdit::timeChanged, this, &TvShowWidgetEpisode::onEpBookmarkChange);
    connect(ui->playCount, elchOverload<int>(&QSpinBox::valueChanged), this, &TvShowWidgetEpisode::onPlayCountChange);
    connect(ui->lastPlayed, &QDateTimeEdit::dateTimeChanged, this, &TvShowWidgetEpisode::onLastPlayedChange);
    connect(ui->studio, &QLineEdit::textEdited, this, &TvShowWidgetEpisode::onStudioChange);
    connect(ui->overview, &QTextEdit::textChanged, this, &TvShowWidgetEpisode::onOverviewChange);
    connect(ui->videoAspectRatio, elchOverload<double>(&QDoubleSpinBox::valueChanged), this, [this](double /*unused*/) {
        onStreamDetailsEdited();
    });
    connect(
        ui->videoCodec, &QLineEdit::textEdited, this, [this](const QString& /*unused*/) { onStreamDetailsEdited(); });
    connect(ui->videoDuration, &QDateTimeEdit::timeChanged, this, [this](const QTime& /*unused*/) {
        onStreamDetailsEdited();
    });
    connect(ui->videoHeight, elchOverload<int>(&QSpinBox::valueChanged), this, [this](int /*unused*/) {
        onStreamDetailsEdited();
    });
    connect(ui->videoWidth, elchOverload<int>(&QSpinBox::valueChanged), this, [this](int /*unused*/) {
        onStreamDetailsEdited();
    });
    connect(ui->videoScantype, &QLineEdit::textEdited, this, [this]() { onStreamDetailsEdited(); });
    connect(ui->stereoMode, elchOverload<int>(&QComboBox::currentIndexChanged), this, [this](int /*unused*/) {
        onStreamDetailsEdited();
    });
    connect(ui->actors, &QTableWidget::itemChanged, this, &TvShowWidgetEpisode::onActorEdited);
    connect(ui->directors, &QTableWidget::itemChanged, this, &TvShowWidgetEpisode::onDirectorEdited);
    connect(ui->writers, &QTableWidget::itemChanged, this, &TvShowWidgetEpisode::onWriterEdited);

    m_loadingMovie = new QMovie(":/img/spinner.gif", QByteArray(), this);
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

    ui->missingLabel->setVisible(false);

    helper::applyStyle(ui->tabWidget);
    helper::applyStyle(ui->labelThumbnail);
    helper::applyEffect(ui->groupBox_3);
    helper::fillStereoModeCombo(ui->stereoMode);
}

/**
 * \brief TvShowWidgetEpisode::~TvShowWidgetEpisode
 */
TvShowWidgetEpisode::~TvShowWidgetEpisode()
{
    delete ui;
}

/**
 * \brief Repositions the saving widget
 */
void TvShowWidgetEpisode::resizeEvent(QResizeEvent* event)
{
    m_savingWidget->move(size().width() / 2 - m_savingWidget->width(), height() / 2 - m_savingWidget->height());
    QWidget::resizeEvent(event);
}

/**
 * \brief Clears all contents
 */
void TvShowWidgetEpisode::onClear()
{
    ui->directors->setRowCount(0);
    ui->writers->setRowCount(0);
    ui->thumbnail->clear();

    bool blocked = ui->episodeName->blockSignals(true);
    ui->episodeName->clear();
    ui->episodeName->blockSignals(blocked);

    ui->files->clear();
    ui->files->setToolTip("");

    blocked = ui->imdbId->blockSignals(true);
    ui->imdbId->clear();
    ui->imdbId->blockSignals(blocked);

    blocked = ui->tvdbId->blockSignals(true);
    ui->tvdbId->clear();
    ui->tvdbId->blockSignals(blocked);

    blocked = ui->tvmazeId->blockSignals(true);
    ui->tvmazeId->clear();
    ui->tvmazeId->blockSignals(blocked);

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

    blocked = ui->votes->blockSignals(true);
    ui->votes->clear();
    ui->votes->blockSignals(blocked);

    blocked = ui->top250->blockSignals(true);
    ui->top250->clear();
    ui->top250->blockSignals(blocked);

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

    blocked = ui->stereoMode->blockSignals(true);
    ui->stereoMode->setCurrentIndex(0);
    ui->stereoMode->blockSignals(blocked);

    blocked = ui->epBookmark->blockSignals(true);
    ui->epBookmark->setTime(QTime(0, 0, 0));
    ui->epBookmark->blockSignals(blocked);

    blocked = ui->tagCloud->blockSignals(true);
    ui->tagCloud->clear();
    ui->tagCloud->blockSignals(blocked);

    ui->actors->setRowCount(0);

    ui->buttonRevert->setVisible(false);
    ui->mediaFlags->clear();
}

/**
 * \brief Sets the enabled status of the main group box
 * \param enabled Status
 */
void TvShowWidgetEpisode::onSetEnabled(bool enabled)
{
    if ((m_episode != nullptr) && m_episode->isDummy()) {
        ui->groupBox_3->setEnabled(false);
        return;
    }
    ui->groupBox_3->setEnabled(enabled);
}

/**
 * \brief Sets the episode and tells the episode to load its images
 * \param episode Episode to set
 */
void TvShowWidgetEpisode::setEpisode(TvShowEpisode* episode)
{
    qDebug() << "Entered, episode=" << episode->title();
    m_episode = episode;
    if (!episode->streamDetailsLoaded() && Settings::instance()->autoLoadStreamDetails() && !episode->isDummy()) {
        // Loading stream details als marks the episode as changed...
        // TODO: Refactor the "hasChanged" stuff...
        const bool prevState = episode->hasChanged();
        episode->loadStreamDetailsFromFile();
        episode->setChanged(prevState);
    }
    ui->missingLabel->setVisible(episode->isDummy());
    updateEpisodeInfo();

    emit sigSetActionSearchEnabled(!episode->isDummy(), MainWidgets::TvShows);
    emit sigSetActionSaveEnabled(!episode->isDummy(), MainWidgets::TvShows);
}

/**
 * \brief Updates the widgets contents with the episode info
 */
void TvShowWidgetEpisode::updateEpisodeInfo()
{
    if (m_episode == nullptr) {
        qWarning() << "My episode is invalid";
        return;
    }

    ui->season->blockSignals(true);
    ui->episode->blockSignals(true);
    ui->displaySeason->blockSignals(true);
    ui->displayEpisode->blockSignals(true);
    ui->rating->blockSignals(true);
    ui->votes->blockSignals(true);
    ui->top250->blockSignals(true);
    ui->certification->blockSignals(true);
    ui->firstAired->blockSignals(true);
    ui->playCount->blockSignals(true);
    ui->lastPlayed->blockSignals(true);
    ui->overview->blockSignals(true);
    ui->epBookmark->blockSignals(true);
    ui->tagCloud->blockSignals(true);

    onClear();

    const QStringList nativeFileList = m_episode->files().toNativeStringList();
    ui->files->setText(nativeFileList.join(", "));
    ui->files->setToolTip(nativeFileList.join("\n"));

    ui->imdbId->setText(m_episode->imdbId().toString());
    ui->tvdbId->setText(m_episode->tvdbId().toString());
    ui->tvmazeId->setText(m_episode->tvmazeId().toString());
    ui->name->setText(m_episode->title());
    ui->showTitle->setText(m_episode->showTitle());
    ui->season->setValue(m_episode->seasonNumber().toInt());
    ui->episode->setValue(m_episode->episodeNumber().toInt());
    ui->displaySeason->setValue(m_episode->displaySeason().toInt());
    ui->displayEpisode->setValue(m_episode->displayEpisode().toInt());

    if (!m_episode->ratings().isEmpty()) {
        ui->rating->setValue(m_episode->ratings().first().rating);
        ui->votes->setValue(m_episode->ratings().first().voteCount);
    } else {
        ui->rating->setValue(0.0);
        ui->votes->setValue(0.0);
    }

    ui->top250->setValue(m_episode->top250());
    ui->firstAired->setDate(m_episode->firstAired());
    ui->playCount->setValue(m_episode->playCount());
    ui->lastPlayed->setDateTime(m_episode->lastPlayed());
    ui->studio->setText(m_episode->network());
    ui->overview->setPlainText(m_episode->overview());
    ui->epBookmark->setTime(m_episode->epBookmark());

    ui->writers->blockSignals(true);
    for (QString* writer : m_episode->writersPointer()) {
        int row = ui->writers->rowCount();
        ui->writers->insertRow(row);
        ui->writers->setItem(row, 0, new QTableWidgetItem(*writer));
        ui->writers->item(row, 0)->setData(Qt::UserRole, QVariant::fromValue(writer));
    }
    ui->writers->blockSignals(false);

    ui->directors->blockSignals(true);
    for (QString* director : m_episode->directorsPointer()) {
        int row = ui->directors->rowCount();
        ui->directors->insertRow(row);
        ui->directors->setItem(row, 0, new QTableWidgetItem(*director));
        ui->directors->item(row, 0)->setData(Qt::UserRole, QVariant::fromValue(director));
    }
    ui->writers->blockSignals(false);

    ui->actors->blockSignals(true);
    for (Actor* actor : m_episode->actors()) {
        const int row = ui->actors->rowCount();
        ui->actors->insertRow(row);
        ui->actors->setItem(row, 0, new QTableWidgetItem(actor->name));
        ui->actors->setItem(row, 1, new QTableWidgetItem(actor->role));
        ui->actors->item(row, 0)->setData(Qt::UserRole, actor->thumb);
        ui->actors->item(row, 1)->setData(Qt::UserRole, QVariant::fromValue(actor));
    }
    ui->actors->blockSignals(false);

    if (m_episode->tvShow() != nullptr) {
        auto certifications = m_episode->tvShow()->certifications();
        certifications.prepend(Certification::NoCertification);
        for (const auto& cert : certifications) {
            ui->certification->addItem(cert.toString());
        }
        ui->certification->setCurrentIndex(certifications.indexOf(m_episode->certification()));
    } else {
        ui->certification->addItem(m_episode->certification().toString());
    }

    // TODO: List of all available tags as first argument
    ui->tagCloud->setTags(m_episode->tags(), m_episode->tags());

    // Streamdetails
    updateStreamDetails();
    ui->videoAspectRatio->setEnabled(m_episode->streamDetailsLoaded());
    ui->videoCodec->setEnabled(m_episode->streamDetailsLoaded());
    ui->videoDuration->setEnabled(m_episode->streamDetailsLoaded());
    ui->videoHeight->setEnabled(m_episode->streamDetailsLoaded());
    ui->videoWidth->setEnabled(m_episode->streamDetailsLoaded());
    ui->videoScantype->setEnabled(m_episode->streamDetailsLoaded());
    ui->stereoMode->setEnabled(m_episode->streamDetailsLoaded());

    if (!m_episode->thumbnailImage().isNull()) {
        ui->thumbnail->setImage(m_episode->thumbnailImage());
    } else if (!Manager::instance()
                    ->mediaCenterInterface()
                    ->imageFileName(m_episode, ImageType::TvShowEpisodeThumb)
                    .isEmpty()) {
        ui->thumbnail->setImage(
            Manager::instance()->mediaCenterInterface()->imageFileName(m_episode, ImageType::TvShowEpisodeThumb));
    }

    ui->season->blockSignals(false);
    ui->episode->blockSignals(false);
    ui->displaySeason->blockSignals(false);
    ui->displayEpisode->blockSignals(false);
    ui->rating->blockSignals(false);
    ui->votes->blockSignals(false);
    ui->top250->blockSignals(false);
    ui->certification->blockSignals(false);
    ui->firstAired->blockSignals(false);
    ui->playCount->blockSignals(false);
    ui->lastPlayed->blockSignals(false);
    ui->overview->blockSignals(false);
    ui->epBookmark->blockSignals(false);
    ui->tagCloud->blockSignals(false);

    ui->buttonRevert->setVisible(m_episode->hasChanged());
}

/**
 * \brief Fills the widget with streamdetails
 * \param reloadFromFile If true forces a reload of streamdetails from the file
 */
void TvShowWidgetEpisode::updateStreamDetails(bool reloadFromFile)
{
    if ((m_episode != nullptr) && m_episode->isDummy()) {
        return;
    }

    ui->videoAspectRatio->blockSignals(true);
    ui->videoDuration->blockSignals(true);
    ui->videoWidth->blockSignals(true);
    ui->videoHeight->blockSignals(true);
    ui->stereoMode->blockSignals(true);

    if (reloadFromFile) {
        m_episode->loadStreamDetailsFromFile();
    }

    StreamDetails* streamDetails = m_episode->streamDetails();
    const auto videoDetails = streamDetails->videoDetails();
    ui->videoWidth->setValue(videoDetails.value(StreamDetails::VideoDetails::Width).toInt());
    ui->videoHeight->setValue(videoDetails.value(StreamDetails::VideoDetails::Height).toInt());
    ui->videoAspectRatio->setValue(
        QString{videoDetails.value(StreamDetails::VideoDetails::Aspect)}.replace(",", ".").toDouble());
    ui->videoCodec->setText(videoDetails.value(StreamDetails::VideoDetails::Codec));
    ui->videoScantype->setText(videoDetails.value(StreamDetails::VideoDetails::ScanType));
    ui->stereoMode->setCurrentIndex(0);
    for (int i = 0, n = ui->stereoMode->count(); i < n; ++i) {
        if (ui->stereoMode->itemData(i).toString() == videoDetails.value(StreamDetails::VideoDetails::StereoMode)) {
            ui->stereoMode->setCurrentIndex(i);
        }
    }
    QTime time(0, 0, 0, 0);
    time = time.addSecs(videoDetails.value(StreamDetails::VideoDetails::DurationInSeconds).toInt());
    ui->videoDuration->setTime(time);

    for (QWidget* widget : m_streamDetailsWidgets) {
        widget->deleteLater();
    }
    m_streamDetailsWidgets.clear();
    m_streamDetailsAudio.clear();
    m_streamDetailsSubtitles.clear();

    int audioTracks = streamDetails->audioDetails().count();
    const auto audioDetails = streamDetails->audioDetails();
    for (int i = 0; i < audioTracks; ++i) {
        auto* label = new QLabel(tr("Track %1").arg(i + 1));
        ui->streamDetails->addWidget(label, 8 + i, 0);
        auto* edit1 = new QLineEdit(audioDetails.at(i).value(StreamDetails::AudioDetails::Language));
        auto* edit2 = new QLineEdit(audioDetails.at(i).value(StreamDetails::AudioDetails::Codec));
        auto* edit3 = new QLineEdit(audioDetails.at(i).value(StreamDetails::AudioDetails::Channels));
        edit3->setMaximumWidth(50);
        edit1->setToolTip(tr("Language"));
        edit2->setToolTip(tr("Codec"));
        edit3->setToolTip(tr("Channels"));
        edit1->setPlaceholderText(tr("Language"));
        edit2->setPlaceholderText(tr("Codec"));
        edit3->setPlaceholderText(tr("Channels"));
        auto* layout = new QHBoxLayout();
        layout->addWidget(edit1);
        layout->addWidget(edit2);
        layout->addWidget(edit3);
        layout->addStretch(10);
        ui->streamDetails->addLayout(layout, 8 + i, 1);
        m_streamDetailsWidgets << label << edit1 << edit2 << edit3;
        m_streamDetailsAudio << (QVector<QLineEdit*>() << edit1 << edit2 << edit3);
        connect(edit1, &QLineEdit::textEdited, this, &TvShowWidgetEpisode::onStreamDetailsEdited);
        connect(edit2, &QLineEdit::textEdited, this, &TvShowWidgetEpisode::onStreamDetailsEdited);
        connect(edit3, &QLineEdit::textEdited, this, &TvShowWidgetEpisode::onStreamDetailsEdited);
    }

    if (!streamDetails->subtitleDetails().isEmpty()) {
        auto* subtitleLabel = new QLabel(tr("Subtitles"));
        QFont font = ui->labelStreamDetailsAudio->font();
        font.setBold(true);
        subtitleLabel->setFont(font);
        ui->streamDetails->addWidget(subtitleLabel, 8 + audioTracks, 0);
        m_streamDetailsWidgets << subtitleLabel;

        for (int i = 0, n = streamDetails->subtitleDetails().count(); i < n; ++i) {
            auto* trackLabel = new QLabel(tr("Track %1").arg(i + 1));
            ui->streamDetails->addWidget(trackLabel, 9 + audioTracks + i, 0);
            auto* edit1 =
                new QLineEdit(streamDetails->subtitleDetails().at(i).value(StreamDetails::SubtitleDetails::Language));
            edit1->setToolTip(tr("Language"));
            edit1->setPlaceholderText(tr("Language"));
            auto* layout = new QHBoxLayout();
            layout->addWidget(edit1);
            layout->addStretch(10);
            ui->streamDetails->addLayout(layout, 9 + audioTracks + i, 1);
            m_streamDetailsWidgets << trackLabel << edit1;
            m_streamDetailsSubtitles << (QVector<QLineEdit*>() << edit1);
            connect(edit1, &QLineEdit::textEdited, this, &TvShowWidgetEpisode::onStreamDetailsEdited);
        }
    }

    // Media Flags
    ui->mediaFlags->setStreamDetails(streamDetails);

    ui->videoAspectRatio->blockSignals(false);
    ui->videoDuration->blockSignals(false);
    ui->videoWidth->blockSignals(false);
    ui->videoHeight->blockSignals(false);
    ui->stereoMode->blockSignals(false);
}

/**
 * \brief Forces a reload of stream details
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
    ui->stereoMode->setEnabled(true);
}

void TvShowWidgetEpisode::onSaveInformation()
{
    if (m_episode == nullptr) {
        qCritical() << "My episode is invalid";
        return;
    }

    if (m_episode->isDummy()) {
        // don't save dummys
        return;
    }

    onSetEnabled(false);
    m_savingWidget->show();
    m_episode->saveData(Manager::instance()->mediaCenterInterfaceTvShow());
    onSetEnabled(true);
    m_savingWidget->hide();
    ui->buttonRevert->setVisible(false);
    NotificationBox::instance()->showSuccess(tr("Episode Saved"));
}

/**
 * \brief Reverts changes
 */
void TvShowWidgetEpisode::onRevertChanges()
{
    m_episode->loadData(Manager::instance()->mediaCenterInterfaceTvShow(), true, true);
    updateEpisodeInfo();
}

/**
 * \brief Shows the search widget
 */
void TvShowWidgetEpisode::onStartScraperSearch()
{
    if (m_episode == nullptr) {
        qWarning() << "My episode is invalid";
        return;
    }

    if (m_episode->isDummy()) {
        return;
    }

    emit sigSetActionSearchEnabled(false, MainWidgets::TvShows);
    emit sigSetActionSaveEnabled(false, MainWidgets::TvShows);

    auto* searchWidget = new TvShowSearch(this);
    searchWidget->setSearchType(TvShowType::Episode);
    searchWidget->execWithSearch(m_episode->showTitle());

    if (searchWidget->result() == QDialog::Accepted) {
        onSetEnabled(false);
        connect(
            m_episode.data(), &TvShowEpisode::sigLoaded, this, &TvShowWidgetEpisode::onLoadDone, Qt::UniqueConnection);

        NotificationBox::instance()->showProgressBar(
            tr("Scraping episode..."), Constants::TvShowScrapeProgressMessageId);

        m_episode->scrapeData(searchWidget->scraper(),
            searchWidget->locale(),
            mediaelch::scraper::ShowIdentifier(searchWidget->showIdentifier()),
            searchWidget->seasonOrder(),
            searchWidget->episodeDetailsToLoad());
    } else {
        emit sigSetActionSearchEnabled(true, MainWidgets::TvShows);
        emit sigSetActionSaveEnabled(true, MainWidgets::TvShows);
    }

    searchWidget->deleteLater();
}

/**
 * \brief Called when the search widget finishes
 * Updates infos and starts downloads
 */
void TvShowWidgetEpisode::onLoadDone()
{
    NotificationBox::instance()->hideProgressBar(Constants::TvShowScrapeProgressMessageId);

    if (m_episode == nullptr) {
        qWarning() << "My episode is invalid";
        return;
    }

    updateEpisodeInfo();
    onSetEnabled(true);

    if (!m_episode->thumbnail().isEmpty() && m_episode->wantThumbnailDownload()) {
        DownloadManagerElement d;
        d.imageType = ImageType::TvShowEpisodeThumb;
        d.url = m_episode->thumbnail();
        d.episode = m_episode;
        d.directDownload = true;
        m_posterDownloadManager->addDownload(d);
        ui->thumbnail->setLoading(true);
    } else {
        emit sigSetActionSearchEnabled(true, MainWidgets::TvShows);
        emit sigSetActionSaveEnabled(true, MainWidgets::TvShows);
    }
    ui->buttonRevert->setVisible(true);
}

/**
 * \brief Shows the MovieImageDialog and after successful execution starts downloads
 */
void TvShowWidgetEpisode::onChooseThumbnail()
{
    if (m_episode == nullptr) {
        qWarning() << "My episode is invalid";
        return;
    }

    auto* imageDialog = new ImageDialog(this);
    imageDialog->setImageType(ImageType::TvShowEpisodeThumb);
    imageDialog->setTvShowEpisode(m_episode);
    QVector<Poster> posters;
    if (!m_episode->thumbnail().isEmpty()) {
        Poster p;
        p.originalUrl = m_episode->thumbnail();
        p.thumbUrl = m_episode->thumbnail();
        posters << p;
    }
    imageDialog->setDefaultDownloads(posters);

    imageDialog->execWithType(ImageType::TvShowEpisodeThumb);
    const int exitCode = imageDialog->result();
    const QUrl imageUrl = imageDialog->imageUrl();
    imageDialog->deleteLater();

    if (exitCode == QDialog::Accepted) {
        emit sigSetActionSaveEnabled(false, MainWidgets::TvShows);
        DownloadManagerElement d;
        d.imageType = ImageType::TvShowEpisodeThumb;
        d.url = imageUrl;
        d.episode = m_episode;
        d.directDownload = true;
        m_posterDownloadManager->addDownload(d);
        ui->thumbnail->setLoading(true);
        ui->buttonRevert->setVisible(true);
    }
}

void TvShowWidgetEpisode::onImageDropped(ImageType imageType, QUrl imageUrl)
{
    Q_UNUSED(imageType);

    if (m_episode == nullptr) {
        return;
    }
    emit sigSetActionSaveEnabled(false, MainWidgets::TvShows);
    DownloadManagerElement d;
    d.imageType = ImageType::TvShowEpisodeThumb;
    d.url = imageUrl;
    d.episode = m_episode;
    d.directDownload = true;
    m_posterDownloadManager->addDownload(d);
    ui->thumbnail->setLoading(true);
    ui->buttonRevert->setVisible(true);
}

/**
 * \brief Adjusts the size of the backdrop to common values (1080p or 720p) and shows the image
 * \param elem Downloaded element
 */
void TvShowWidgetEpisode::onPosterDownloadFinished(DownloadManagerElement elem)
{
    if (elem.imageType == ImageType::TvShowEpisodeThumb) {
        qDebug() << "Got a backdrop";
        if (m_episode == elem.episode) {
            ui->thumbnail->setImage(elem.data);
        }
        ImageCache::instance()->invalidateImages(
            Manager::instance()->mediaCenterInterface()->imageFileName(elem.episode, ImageType::TvShowEpisodeThumb));
        elem.episode->setThumbnailImage(elem.data);
    }
    if (m_posterDownloadManager->downloadQueueSize() == 0) {
        emit sigSetActionSaveEnabled(true, MainWidgets::TvShows);
        emit sigSetActionSearchEnabled(true, MainWidgets::TvShows);
    }
}

/*** add/remove/edit Actors, Genres, Countries and Studios ***/

/**
 * \brief Adds a director
 */
void TvShowWidgetEpisode::onAddDirector()
{
    QString d = tr("Unknown Director");
    m_episode->addDirector(d);
    QString* director = m_episode->directorsPointer().last();

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
 * \brief Removes a director
 */
void TvShowWidgetEpisode::onRemoveDirector()
{
    int row = ui->directors->currentRow();
    if (row < 0 || row >= ui->directors->rowCount() || !ui->directors->currentItem()->isSelected()) {
        return;
    }

    auto* director = ui->directors->item(row, 0)->data(Qt::UserRole).value<QString*>();
    m_episode->removeDirector(director);
    ui->directors->blockSignals(true);
    ui->directors->removeRow(row);
    ui->directors->blockSignals(false);
    ui->buttonRevert->setVisible(true);
}

/**
 * \brief Stores the changed values
 * \param item Edited item
 */
void TvShowWidgetEpisode::onDirectorEdited(QTableWidgetItem* item)
{
    auto* director = ui->directors->item(item->row(), 0)->data(Qt::UserRole).value<QString*>();
    director->clear();
    director->append(item->text());
    m_episode->setChanged(true);
    ui->buttonRevert->setVisible(true);
}

/**
 * \brief Adds a writer
 */
void TvShowWidgetEpisode::onAddWriter()
{
    QString w = tr("Unknown Writer");
    m_episode->addWriter(w);
    QString* writer = m_episode->writersPointer().last();

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
 * \brief Removes a writer
 */
void TvShowWidgetEpisode::onRemoveWriter()
{
    int row = ui->writers->currentRow();
    if (row < 0 || row >= ui->writers->rowCount() || !ui->writers->currentItem()->isSelected()) {
        return;
    }

    auto writer = ui->writers->item(row, 0)->data(Qt::UserRole).value<QString*>();
    m_episode->removeWriter(writer);
    ui->writers->blockSignals(true);
    ui->writers->removeRow(row);
    ui->writers->blockSignals(false);
    ui->buttonRevert->setVisible(true);
}

/**
 * \brief Stores the changed values
 * \param item Edited item
 */
void TvShowWidgetEpisode::onWriterEdited(QTableWidgetItem* item)
{
    auto writer = ui->writers->item(item->row(), 0)->data(Qt::UserRole).value<QString*>();
    writer->clear();
    writer->append(item->text());
    m_episode->setChanged(true);
    ui->buttonRevert->setVisible(true);
}

/*** Pass GUI events to episode object ***/

void TvShowWidgetEpisode::onImdbIdChanged(QString imdbid)
{
    m_episode->setImdbId(ImdbId(imdbid));
    ui->buttonRevert->setVisible(true);
}

void TvShowWidgetEpisode::onTvdbIdChanged(QString tvdbid)
{
    m_episode->setTvdbId(TvDbId(tvdbid));
    ui->buttonRevert->setVisible(true);
}

void TvShowWidgetEpisode::onTvmazeIdChanged(QString tvmazeId)
{
    m_episode->setTvMazeId(TvMazeId(tvmazeId));
    ui->buttonRevert->setVisible(true);
}

/**
 * \brief Marks the episode as changed when the name has changed
 */
void TvShowWidgetEpisode::onNameChange(QString text)
{
    m_episode->setTitle(std::move(text));
    ui->buttonRevert->setVisible(true);
}

/**
 * \brief Marks the episode as changed when the show title has changed
 */
void TvShowWidgetEpisode::onShowTitleChange(QString text)
{
    m_episode->setShowTitle(std::move(text));
    ui->buttonRevert->setVisible(true);
}

/**
 * \brief Marks the episode as changed when the season has changed
 */
void TvShowWidgetEpisode::onSeasonChange(int value)
{
    m_episode->setSeason(SeasonNumber(value));
    ui->buttonRevert->setVisible(true);
}

/**
 * \brief Marks the episode as changed when the episode has changed
 */
void TvShowWidgetEpisode::onEpisodeChange(int value)
{
    m_episode->setEpisode(EpisodeNumber(value));
    ui->buttonRevert->setVisible(true);
}

/**
 * \brief Marks the episode as changed when the display season has changed
 */
void TvShowWidgetEpisode::onDisplaySeasonChange(int value)
{
    m_episode->setDisplaySeason(SeasonNumber(value));
    ui->buttonRevert->setVisible(true);
}

/**
 * \brief Marks the episode as changed when the display episode has changed
 */
void TvShowWidgetEpisode::onDisplayEpisodeChange(int value)
{
    m_episode->setDisplayEpisode(EpisodeNumber(value));
    ui->buttonRevert->setVisible(true);
}

void TvShowWidgetEpisode::onRatingChange(double value)
{
    auto& ratings = m_episode->ratings();
    if (ratings.isEmpty()) {
        ratings.push_back({});
    }

    ratings.first().rating = value;
    m_episode->setChanged(true);
    ui->buttonRevert->setVisible(true);
}

/**
 * \brief Marks the episode as changed when the overview has changed
 */
void TvShowWidgetEpisode::onCertificationChange(QString text)
{
    m_episode->setCertification(Certification(text));
    ui->buttonRevert->setVisible(true);
}

/**
 * \brief Marks the episode as changed when the first aired date has changed
 */
void TvShowWidgetEpisode::onFirstAiredChange(QDate date)
{
    m_episode->setFirstAired(date);
    ui->buttonRevert->setVisible(true);
}

/**
 * \brief Marks the episode as changed when the playcount has changed
 */
void TvShowWidgetEpisode::onPlayCountChange(int value)
{
    m_episode->setPlayCount(value);
    ui->buttonRevert->setVisible(true);
}

/**
 * \brief Marks the episode as changed when the last played date has changed
 */
void TvShowWidgetEpisode::onLastPlayedChange(QDateTime dateTime)
{
    m_episode->setLastPlayed(dateTime);
    ui->buttonRevert->setVisible(true);
}

/**
 * \brief Marks the episode as changed when the studio has changed
 */
void TvShowWidgetEpisode::onStudioChange(QString text)
{
    m_episode->setNetwork(text);
    ui->buttonRevert->setVisible(true);
}

void TvShowWidgetEpisode::onEpBookmarkChange(QTime time)
{
    if (m_episode == nullptr) {
        return;
    }
    m_episode->setEpBookmark(time);
    ui->buttonRevert->setVisible(true);
}

/**
 * \brief Marks the episode as changed when the overview has changed
 */
void TvShowWidgetEpisode::onOverviewChange()
{
    m_episode->setOverview(ui->overview->toPlainText());
    ui->buttonRevert->setVisible(true);
}

void TvShowWidgetEpisode::onDeleteThumbnail()
{
    m_episode->removeImage(ImageType::TvShowEpisodeThumb);
    if (!m_episode->imagesToRemove().contains(ImageType::TvShowEpisodeThumb)
        && !Manager::instance()
                ->mediaCenterInterface()
                ->imageFileName(m_episode, ImageType::TvShowEpisodeThumb)
                .isEmpty()) {
        ui->thumbnail->setImage(
            Manager::instance()->mediaCenterInterface()->imageFileName(m_episode, ImageType::TvShowEpisodeThumb));
    }
    ui->buttonRevert->setVisible(true);
}

/**
 * \brief Updates all stream details for this episode with values from the widget
 */
void TvShowWidgetEpisode::onStreamDetailsEdited()
{
    StreamDetails* details = m_episode->streamDetails();
    details->setVideoDetail(StreamDetails::VideoDetails::Codec, ui->videoCodec->text());
    details->setVideoDetail(StreamDetails::VideoDetails::Aspect, ui->videoAspectRatio->text());
    details->setVideoDetail(StreamDetails::VideoDetails::Width, ui->videoWidth->text());
    details->setVideoDetail(StreamDetails::VideoDetails::Height, ui->videoHeight->text());
    details->setVideoDetail(StreamDetails::VideoDetails::ScanType, ui->videoScantype->text());
    details->setVideoDetail(StreamDetails::VideoDetails::DurationInSeconds,
        QString("%1").arg(-ui->videoDuration->time().secsTo(QTime(0, 0))));
    details->setVideoDetail(StreamDetails::VideoDetails::StereoMode, ui->stereoMode->currentData().toString());

    for (int i = 0, n = m_streamDetailsAudio.count(); i < n; ++i) {
        details->setAudioDetail(i, StreamDetails::AudioDetails::Language, m_streamDetailsAudio[i][0]->text());
        details->setAudioDetail(i, StreamDetails::AudioDetails::Codec, m_streamDetailsAudio[i][1]->text());
        details->setAudioDetail(i, StreamDetails::AudioDetails::Channels, m_streamDetailsAudio[i][2]->text());
    }
    for (int i = 0, n = m_streamDetailsSubtitles.count(); i < n; ++i) {
        details->setSubtitleDetail(i, StreamDetails::SubtitleDetails::Language, m_streamDetailsSubtitles[i][0]->text());
    }

    m_episode->setChanged(true);
    ui->buttonRevert->setVisible(true);
}

void TvShowWidgetEpisode::onAddTag(QString tag)
{
    if (m_episode == nullptr) {
        return;
    }
    m_episode->addTag(tag);
    ui->buttonRevert->setVisible(true);
}

void TvShowWidgetEpisode::onRemoveTag(QString tag)
{
    if (m_episode == nullptr) {
        return;
    }
    m_episode->removeTag(tag);
    ui->buttonRevert->setVisible(true);
}

void TvShowWidgetEpisode::onActorEdited(QTableWidgetItem* item)
{
    auto actor = ui->actors->item(item->row(), 1)->data(Qt::UserRole).value<Actor*>();
    if (item->column() == 0) {
        actor->name = item->text();
    } else if (item->column() == 1) {
        actor->role = item->text();
    }
    m_episode->setChanged(true);
    ui->buttonRevert->setVisible(true);
}

void TvShowWidgetEpisode::onAddActor()
{
    Actor a;
    a.name = tr("Unknown Actor");
    a.role = tr("Unknown Role");
    m_episode->addActor(a);

    Actor* actor = m_episode->actors().back();

    ui->actors->blockSignals(true);
    int row = ui->actors->rowCount();
    ui->actors->insertRow(row);
    ui->actors->setItem(row, 0, new QTableWidgetItem(actor->name));
    ui->actors->setItem(row, 1, new QTableWidgetItem(actor->role));
    ui->actors->item(row, 1)->setData(Qt::UserRole, QVariant::fromValue(actor));
    ui->actors->scrollToBottom();
    ui->actors->blockSignals(false);
    ui->buttonRevert->setVisible(true);
}

void TvShowWidgetEpisode::onRemoveActor()
{
    int row = ui->actors->currentRow();
    if (row < 0 || row >= ui->actors->rowCount() || !ui->actors->currentItem()->isSelected()) {
        return;
    }

    auto actor = ui->actors->item(row, 1)->data(Qt::UserRole).value<Actor*>();
    m_episode->removeActor(actor);
    ui->actors->blockSignals(true);
    ui->actors->removeRow(row);
    ui->actors->blockSignals(false);
    ui->buttonRevert->setVisible(true);
}

void TvShowWidgetEpisode::onActorChanged()
{
    if (ui->actors->currentRow() < 0 || ui->actors->currentRow() >= ui->actors->rowCount()
        || ui->actors->currentColumn() < 0 || ui->actors->currentColumn() >= ui->actors->colorCount()) {
        ui->actor->setPixmap(QPixmap(":/img/man.png"));
        ui->actorResolution->setText("");
        return;
    }

    auto actor = ui->actors->item(ui->actors->currentRow(), 1)->data(Qt::UserRole).value<Actor*>();
    if (!actor->image.isNull()) {
        QImage img = QImage::fromData(actor->image);
        ui->actor->setPixmap(QPixmap::fromImage(img).scaled(120, 180, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        ui->actorResolution->setText(QString("%1 x %2").arg(img.width()).arg(img.height()));
    } else if (!Manager::instance()->mediaCenterInterface()->actorImageName(m_episode, *actor).isEmpty()) {
        QPixmap p(Manager::instance()->mediaCenterInterface()->actorImageName(m_episode, *actor));
        ui->actor->setPixmap(p.scaled(120, 180, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        ui->actorResolution->setText(QString("%1 x %2").arg(p.width()).arg(p.height()));
    } else {
        ui->actor->setPixmap(QPixmap(":/img/man.png"));
        ui->actorResolution->setText("");
    }
}

void TvShowWidgetEpisode::onChangeActorImage()
{
    if (ui->actors->currentRow() < 0 || ui->actors->currentRow() >= ui->actors->rowCount()
        || ui->actors->currentColumn() < 0 || ui->actors->currentColumn() >= ui->actors->colorCount()) {
        return;
    }

    QString fileName =
        QFileDialog::getOpenFileName(parentWidget(), tr("Choose Image"), QDir::homePath(), tr("Images (*.jpg *.jpeg)"));
    if (!fileName.isNull()) {
        QImage img(fileName);
        if (!img.isNull()) {
            QByteArray ba;
            QBuffer buffer(&ba);
            img.save(&buffer, "jpg", 100);
            auto actor = ui->actors->item(ui->actors->currentRow(), 1)->data(Qt::UserRole).value<Actor*>();
            actor->image = ba;
            actor->imageHasChanged = true;
            onActorChanged();
            m_episode->setChanged(true);
        }
        ui->buttonRevert->setVisible(true);
    }
}

void TvShowWidgetEpisode::onVotesChange(int value)
{
    if (m_episode == nullptr) {
        return;
    }

    auto& ratings = m_episode->ratings();
    if (ratings.isEmpty()) {
        ratings.push_back({});
    }

    ratings.first().voteCount = value;
    m_episode->setChanged(true);
    ui->buttonRevert->setVisible(true);
}

void TvShowWidgetEpisode::onTop250Change(int value)
{
    if (m_episode == nullptr) {
        return;
    }
    m_episode->setTop250(value);
    ui->buttonRevert->setVisible(true);
}

void TvShowWidgetEpisode::onCaptureImage(ImageType type)
{
    using namespace mediaelch;

    Q_UNUSED(type)
    if ((m_episode == nullptr) || m_episode->files().isEmpty()) {
        return;
    }
    auto dimensions = Settings::instance()->advanced()->episodeThumbnailDimensions();
    QImage img;
    if (!ImageCapture::captureImage(m_episode->files().first(), m_episode->streamDetails(), dimensions, img, false)) {
        return;
    }

    QByteArray ba;
    QBuffer buffer(&ba);
    buffer.open(QIODevice::WriteOnly);
    img.save(&buffer, "JPG", 85);

    ui->thumbnail->setImage(ba);
    ImageCache::instance()->invalidateImages(
        Manager::instance()->mediaCenterInterface()->imageFileName(m_episode, ImageType::TvShowEpisodeThumb));
    m_episode->setThumbnailImage(ba);
}
