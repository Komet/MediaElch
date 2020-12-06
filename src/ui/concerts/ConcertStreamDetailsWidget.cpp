#include "ConcertStreamDetailsWidget.h"
#include "ui_ConcertStreamDetailsWidget.h"

#include "concerts/Concert.h"
#include "data/StreamDetails.h"
#include "globals/Helper.h"

#include <QAbstractButton>
#include <QDateTimeEdit>
#include <QLineEdit>
#include <QtMath>

ConcertStreamDetailsWidget::ConcertStreamDetailsWidget(QWidget* parent) :
    QWidget(parent), ui(new Ui::ConcertStreamDetailsWidget)
{
    ui->setupUi(this);
    connect(ui->buttonReloadStreamDetails,
        &QAbstractButton::clicked,
        this,
        &ConcertStreamDetailsWidget::onReloadStreamDetails);

    auto streamDetailsEdited = [this]() { onStreamDetailsEdited(); };

    // clang-format off
    connect(ui->videoCodec,       &QLineEdit::textEdited,           this, streamDetailsEdited);
    connect(ui->videoDuration,    &QDateTimeEdit::timeChanged,      this, streamDetailsEdited);
    connect(ui->videoScantype,    &QLineEdit::textEdited,           this, streamDetailsEdited);
    connect(ui->videoAspectRatio, elchOverload<double>(&QDoubleSpinBox::valueChanged), this, streamDetailsEdited);
    connect(ui->videoHeight,      elchOverload<int>(&QSpinBox::valueChanged),          this, streamDetailsEdited);
    connect(ui->videoWidth,       elchOverload<int>(&QSpinBox::valueChanged),          this, streamDetailsEdited);
    connect(ui->stereoMode,       elchOverload<int>(&QComboBox::currentIndexChanged),  this, streamDetailsEdited);
    // clang-format on

    helper::fillStereoModeCombo(ui->stereoMode);
}

void ConcertStreamDetailsWidget::setConcertController(ConcertController* controller)
{
    m_concertController = controller;
}

void ConcertStreamDetailsWidget::updateConcertInfo()
{
    if ((m_concertController == nullptr) || (m_concertController->concert() == nullptr)) {
        qDebug() << "My concert is invalid";
        return;
    }
    clear();
    updateStreamDetails();
    ui->videoAspectRatio->setEnabled(m_concertController->concert()->streamDetailsLoaded());
    ui->videoCodec->setEnabled(m_concertController->concert()->streamDetailsLoaded());
    ui->videoDuration->setEnabled(m_concertController->concert()->streamDetailsLoaded());
    ui->videoHeight->setEnabled(m_concertController->concert()->streamDetailsLoaded());
    ui->videoWidth->setEnabled(m_concertController->concert()->streamDetailsLoaded());
    ui->videoScantype->setEnabled(m_concertController->concert()->streamDetailsLoaded());
    ui->stereoMode->setEnabled(m_concertController->concert()->streamDetailsLoaded());
}

// Do NOT move the destructor into the header or unique_ptr requires a
// complete type of UI::ConcertStreamDetailsWidget
ConcertStreamDetailsWidget::~ConcertStreamDetailsWidget() = default;

/**
 * \brief Forces a reload of stream details
 */
void ConcertStreamDetailsWidget::onReloadStreamDetails()
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

/**
 * \brief Fills the widget with streamdetails
 * \param reloadFromFile If true forces a reload of streamdetails from the file
 */
void ConcertStreamDetailsWidget::updateStreamDetails(bool reloadFromFile)
{
    ui->videoAspectRatio->blockSignals(true);
    ui->videoDuration->blockSignals(true);
    ui->videoWidth->blockSignals(true);
    ui->videoHeight->blockSignals(true);
    ui->stereoMode->blockSignals(true);

    if (reloadFromFile) {
        m_concertController->loadStreamDetailsFromFile();
    }

    StreamDetails* streamDetails = m_concertController->concert()->streamDetails();
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
    if (reloadFromFile) {
        using namespace std::chrono;
        const seconds runtime{qFloor(videoDetails.value(StreamDetails::VideoDetails::DurationInSeconds).toInt())};
        emit runtimeChanged(duration_cast<minutes>(runtime));
    }

    for (QWidget* widget : m_streamDetailsWidgets) {
        widget->deleteLater();
    }
    m_streamDetailsWidgets.clear();
    m_streamDetailsAudio.clear();
    m_streamDetailsSubtitles.clear();

    const auto audioDetails = streamDetails->audioDetails();
    const int audioTracks = audioDetails.count();
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
        connect(edit1, &QLineEdit::textEdited, this, &ConcertStreamDetailsWidget::onStreamDetailsEdited);
        connect(edit2, &QLineEdit::textEdited, this, &ConcertStreamDetailsWidget::onStreamDetailsEdited);
        connect(edit3, &QLineEdit::textEdited, this, &ConcertStreamDetailsWidget::onStreamDetailsEdited);
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
            connect(edit1, &QLineEdit::textEdited, this, &ConcertStreamDetailsWidget::onStreamDetailsEdited);
        }
    }

    ui->videoAspectRatio->blockSignals(false);
    ui->videoDuration->blockSignals(false);
    ui->videoWidth->blockSignals(false);
    ui->videoHeight->blockSignals(false);
    ui->stereoMode->blockSignals(false);

    emit streamDetailsChanged(); // MediaFlags updated in ConcertWidget
}

void ConcertStreamDetailsWidget::clear()
{
    ui->videoCodec->clear();
    ui->videoScantype->clear();

    bool blocked = false;
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
}

/**
 * \brief Updates all stream details for this concert with values from the widget
 */
void ConcertStreamDetailsWidget::onStreamDetailsEdited()
{
    using VideoDetails = StreamDetails::VideoDetails;
    using AudioDetails = StreamDetails::AudioDetails;
    using SubtitleDetails = StreamDetails::SubtitleDetails;

    StreamDetails* details = m_concertController->concert()->streamDetails();
    details->setVideoDetail(VideoDetails::Codec, ui->videoCodec->text());
    details->setVideoDetail(VideoDetails::Aspect, ui->videoAspectRatio->text());
    details->setVideoDetail(VideoDetails::Width, ui->videoWidth->text());
    details->setVideoDetail(VideoDetails::Height, ui->videoHeight->text());
    details->setVideoDetail(VideoDetails::ScanType, ui->videoScantype->text());
    details->setVideoDetail(
        VideoDetails::DurationInSeconds, QString::number(-ui->videoDuration->time().secsTo(QTime(0, 0))));
    details->setVideoDetail(VideoDetails::StereoMode, ui->stereoMode->currentData().toString());

    for (int i = 0, n = m_streamDetailsAudio.count(); i < n; ++i) {
        details->setAudioDetail(i, AudioDetails::Language, m_streamDetailsAudio[i][0]->text());
        details->setAudioDetail(i, AudioDetails::Codec, m_streamDetailsAudio[i][1]->text());
        details->setAudioDetail(i, AudioDetails::Channels, m_streamDetailsAudio[i][2]->text());
    }
    for (int i = 0, n = m_streamDetailsSubtitles.count(); i < n; ++i) {
        details->setSubtitleDetail(i, SubtitleDetails::Language, m_streamDetailsSubtitles[i][0]->text());
    }

    m_concertController->concert()->setChanged(true);
    emit streamDetailsChanged();
}
