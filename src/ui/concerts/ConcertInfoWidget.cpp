#include "ConcertInfoWidget.h"
#include "ui_ConcertInfoWidget.h"

#include "globals/LocaleStringCompare.h"
#include "globals/Manager.h"

// Each change event listener requires the concert to be valid. This is a marco to avoid repitition.
// do {} while() is used to force a semicolon after the use of this macro.
// NOLINTNEXTLINE
#define ME_REQUIRE_CONCERT_OR_RETURN                                                                                   \
    do {                                                                                                               \
        if (!m_concertController || !m_concertController->concert()) {                                                 \
            return;                                                                                                    \
        }                                                                                                              \
    } while (false)

ConcertInfoWidget::ConcertInfoWidget(QWidget* parent) : QWidget(parent), ui(std::make_unique<Ui::ConcertInfoWidget>())
{
    ui->setupUi(this);

    ui->badgeWatched->setBadgeType(Badge::Type::BadgeInfo);

    // clang-format off
    connect(ui->tmdbId,        &QLineEdit::textEdited,          this, &ConcertInfoWidget::onTmdbIdChanged);
    connect(ui->imdbId,        &QLineEdit::textEdited,          this, &ConcertInfoWidget::onImdbIdChanged);

    connect(ui->title,         &QLineEdit::textChanged,         this, &ConcertInfoWidget::onConcertTitleChanged);
    connect(ui->title,         &QLineEdit::textEdited,          this, &ConcertInfoWidget::onTitleChange);
    connect(ui->originalTitle, &QLineEdit::textEdited,          this, &ConcertInfoWidget::onOriginalTitleChange);
    connect(ui->artist,        &QLineEdit::textEdited,          this, &ConcertInfoWidget::onArtistChange);
    connect(ui->album,         &QLineEdit::textEdited,          this, &ConcertInfoWidget::onAlbumChange);
    connect(ui->tagline,       &QLineEdit::textEdited,          this, &ConcertInfoWidget::onTaglineChange);

    connect(ui->rating,        elchOverload<double>(&QDoubleSpinBox::valueChanged), this, &ConcertInfoWidget::onRatingChange);
    connect(ui->userRating,    elchOverload<double>(&QDoubleSpinBox::valueChanged), this, &ConcertInfoWidget::onUserRatingChange);
    connect(ui->runtime,       elchOverload<int>(&QSpinBox::valueChanged),          this, &ConcertInfoWidget::onRuntimeChange);
    connect(ui->playcount,     elchOverload<int>(&QSpinBox::valueChanged),          this, &ConcertInfoWidget::onPlayCountChange);

    connect(ui->trailer,       &QLineEdit::textEdited,          this, &ConcertInfoWidget::onTrailerChange);
    connect(ui->certification, &QComboBox::editTextChanged,     this, &ConcertInfoWidget::onCertificationChange);
    connect(ui->badgeWatched,  &Badge::clicked,                 this, &ConcertInfoWidget::onWatchedClicked);
    connect(ui->released,      &QDateTimeEdit::dateChanged,     this, &ConcertInfoWidget::onReleasedChange);
    connect(ui->lastPlayed,    &QDateTimeEdit::dateTimeChanged, this, &ConcertInfoWidget::onLastWatchedChange);
    connect(ui->overview,      &QTextEdit::textChanged,         this, &ConcertInfoWidget::onOverviewChange);
    // clang-format on

    ui->rating->setSingleStep(0.1);
    ui->rating->setMinimum(0.0);
    ui->userRating->setSingleStep(0.1);
    ui->userRating->setMinimum(0.0);
}

// Do NOT move the destructor into the header or unique_ptr requires a
// complete type of UI::ConcertInfoWidget
ConcertInfoWidget::~ConcertInfoWidget() = default;

void ConcertInfoWidget::setConcertController(ConcertController* controller)
{
    m_concertController = controller;
}

void ConcertInfoWidget::updateConcertInfo()
{
    if ((m_concertController == nullptr) || (m_concertController->concert() == nullptr)) {
        qDebug() << "My concert is invalid";
        return;
    }

    ui->rating->blockSignals(true);
    ui->userRating->blockSignals(true);
    ui->runtime->blockSignals(true);
    ui->playcount->blockSignals(true);
    ui->certification->blockSignals(true);
    ui->released->blockSignals(true);
    ui->lastPlayed->blockSignals(true);
    ui->overview->blockSignals(true);

    clear();

    const QStringList nativeFileList = m_concertController->concert()->files().toNativeStringList();
    ui->files->setText(nativeFileList.join(", "));
    ui->files->setToolTip(nativeFileList.join("\n"));

    ui->title->setText(m_concertController->concert()->title());
    ui->originalTitle->setText(m_concertController->concert()->originalTitle());
    ui->imdbId->setText(m_concertController->concert()->imdbId().toString());
    ui->tmdbId->setText(m_concertController->concert()->tmdbId().toString());
    ui->artist->setText(m_concertController->concert()->artist());
    ui->album->setText(m_concertController->concert()->album());
    ui->tagline->setText(m_concertController->concert()->tagline());

    if (!m_concertController->concert()->ratings().isEmpty()) {
        ui->rating->setValue(m_concertController->concert()->ratings().first().rating);
    } else {
        ui->rating->setValue(0.0);
    }

    ui->userRating->setValue(m_concertController->concert()->userRating());
    ui->released->setDate(m_concertController->concert()->released());
    ui->runtime->setValue(static_cast<int>(m_concertController->concert()->runtime().count()));
    ui->trailer->setText(m_concertController->concert()->trailer().toString());
    ui->playcount->setValue(m_concertController->concert()->playcount());
    ui->lastPlayed->setDateTime(m_concertController->concert()->lastPlayed());
    ui->overview->setPlainText(m_concertController->concert()->overview());
    ui->badgeWatched->setActive(m_concertController->concert()->watched());

    QStringList certifications;
    certifications.append("");
    for (const Concert* concert : Manager::instance()->concertModel()->concerts()) {
        if (!certifications.contains(concert->certification().toString()) && concert->certification().isValid()) {
            certifications.append(concert->certification().toString());
        }
    }
    std::sort(certifications.begin(), certifications.end(), LocaleStringCompare());
    ui->certification->addItems(certifications);
    ui->certification->setCurrentIndex(
        certifications.indexOf(m_concertController->concert()->certification().toString()));

    ui->rating->blockSignals(false);
    ui->userRating->blockSignals(false);
    ui->runtime->blockSignals(false);
    ui->playcount->blockSignals(false);
    ui->certification->blockSignals(false);
    ui->released->blockSignals(false);
    ui->lastPlayed->blockSignals(false);
    ui->overview->blockSignals(false);
}

void ConcertInfoWidget::setRuntime(std::chrono::minutes runtime)
{
    ui->runtime->setValue(static_cast<int>(runtime.count()));
}

void ConcertInfoWidget::clear()
{
    ui->certification->clear();
    ui->files->clear();
    ui->title->clear();
    ui->originalTitle->clear();
    ui->tmdbId->clear();
    ui->imdbId->clear();
    ui->artist->clear();
    ui->album->clear();
    ui->tagline->clear();
    ui->rating->clear();
    ui->userRating->clear();
    ui->released->setDate(QDate::currentDate());
    ui->runtime->clear();
    ui->trailer->clear();
    ui->playcount->clear();
    ui->lastPlayed->setDateTime(QDateTime::currentDateTime());
    ui->overview->clear();
}

void ConcertInfoWidget::onConcertTitleChanged(QString concertName)
{
    emit concertNameChanged(concertName);
}

void ConcertInfoWidget::onTitleChange(QString text)
{
    ME_REQUIRE_CONCERT_OR_RETURN;
    m_concertController->concert()->setTitle(text);
    emit infoChanged();
}

void ConcertInfoWidget::onOriginalTitleChange(QString text)
{
    ME_REQUIRE_CONCERT_OR_RETURN;
    m_concertController->concert()->setOriginalTitle(text);
    emit infoChanged();
}

void ConcertInfoWidget::onTmdbIdChanged(QString text)
{
    ME_REQUIRE_CONCERT_OR_RETURN;
    m_concertController->concert()->setTmdbId(TmdbId(text));
    emit infoChanged();
}

void ConcertInfoWidget::onImdbIdChanged(QString text)
{
    ME_REQUIRE_CONCERT_OR_RETURN;
    m_concertController->concert()->setImdbId(ImdbId(text));
    emit infoChanged();
}

void ConcertInfoWidget::onArtistChange(QString text)
{
    ME_REQUIRE_CONCERT_OR_RETURN;
    m_concertController->concert()->setArtist(text);
    emit infoChanged();
}

/**
 * \brief Marks the concert as changed when the album has changed
 */
void ConcertInfoWidget::onAlbumChange(QString text)
{
    ME_REQUIRE_CONCERT_OR_RETURN;
    m_concertController->concert()->setAlbum(text);
    emit infoChanged();
}

/**
 * \brief Marks the concert as changed when the tagline has changed
 */
void ConcertInfoWidget::onTaglineChange(QString text)
{
    ME_REQUIRE_CONCERT_OR_RETURN;
    m_concertController->concert()->setTagline(text);
    emit infoChanged();
}

/**
 * \brief Marks the concert as changed when the rating has changed
 */
void ConcertInfoWidget::onRatingChange(double value)
{
    ME_REQUIRE_CONCERT_OR_RETURN;
    auto& ratings = m_concertController->concert()->ratings();

    Rating rating;
    rating.rating = value;

    if (ratings.isEmpty()) {
        ratings.push_back(rating);
    } else {
        ratings.first() = rating;
    }

    m_concertController->concert()->setChanged(true);

    emit infoChanged();
}

/// \brief Marks the concert as changed when the userrating has changed
void ConcertInfoWidget::onUserRatingChange(double value)
{
    ME_REQUIRE_CONCERT_OR_RETURN;
    m_concertController->concert()->setUserRating(value);
    emit infoChanged();
}

/**
 * \brief Marks the concert as changed when the release date has changed
 */
void ConcertInfoWidget::onReleasedChange(QDate date)
{
    ME_REQUIRE_CONCERT_OR_RETURN;
    m_concertController->concert()->setReleased(date);
    emit infoChanged();
}

/**
 * \brief Marks the concert as changed when the runtime has changed
 */
void ConcertInfoWidget::onRuntimeChange(const int value)
{
    ME_REQUIRE_CONCERT_OR_RETURN;
    m_concertController->concert()->setRuntime(std::chrono::minutes(value));
    emit infoChanged();
}

/**
 * \brief Marks the concert as changed when the certification has changed
 */
void ConcertInfoWidget::onCertificationChange(QString text)
{
    ME_REQUIRE_CONCERT_OR_RETURN;
    m_concertController->concert()->setCertification(Certification(text));
    emit infoChanged();
}

/**
 * \brief Marks the concert as changed when the trailer has changed
 */
void ConcertInfoWidget::onTrailerChange(QString text)
{
    ME_REQUIRE_CONCERT_OR_RETURN;
    m_concertController->concert()->setTrailer(text);
    emit infoChanged();
}

void ConcertInfoWidget::onWatchedClicked()
{
    ME_REQUIRE_CONCERT_OR_RETURN;
    const bool active = !ui->badgeWatched->isActive();
    ui->badgeWatched->setActive(active);

    Concert* concert = m_concertController->concert();

    if (active) {
        concert->setPlayCount(std::max(1, concert->playcount()));
        ui->playcount->setValue(concert->playcount());

        if (!concert->lastPlayed().isValid()) {
            ui->lastPlayed->setDateTime(QDateTime::currentDateTime());
        }

    } else {
        concert->setPlayCount(0);
        concert->setLastPlayed(QDateTime{});
        ui->playcount->setValue(0);
    }
    emit infoChanged();
}

/**
 * \brief Marks the concert as changed when the play count has changed
 */
void ConcertInfoWidget::onPlayCountChange(int value)
{
    ME_REQUIRE_CONCERT_OR_RETURN;
    m_concertController->concert()->setPlayCount(value);
    ui->badgeWatched->setActive(value > 0);
    emit infoChanged();
}

/**
 * \brief Marks the concert as changed when the last watched date has changed
 */
void ConcertInfoWidget::onLastWatchedChange(QDateTime dateTime)
{
    ME_REQUIRE_CONCERT_OR_RETURN;
    m_concertController->concert()->setLastPlayed(dateTime);
    emit infoChanged();
}

/**
 * \brief Marks the concert as changed when the overview has changed
 */
void ConcertInfoWidget::onOverviewChange()
{
    ME_REQUIRE_CONCERT_OR_RETURN;
    m_concertController->concert()->setOverview(ui->overview->toPlainText());
    emit infoChanged();
}
