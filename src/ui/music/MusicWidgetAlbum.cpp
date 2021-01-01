#include "MusicWidgetAlbum.h"
#include "ui_MusicWidgetAlbum.h"

#include "globals/Globals.h"
#include "globals/Helper.h"
#include "globals/ImageDialog.h"
#include "globals/Manager.h"
#include "globals/MessageIds.h"
#include "ui/music/MusicSearch.h"
#include "ui/notifications/NotificationBox.h"

#ifdef Q_OS_WIN
#    include <QGLFormat>
#endif
#include <QPainter>

MusicWidgetAlbum::MusicWidgetAlbum(QWidget* parent) : QWidget(parent), ui(new Ui::MusicWidgetAlbum)
{
    ui->setupUi(this);

    m_album = nullptr;
    ui->albumName->clear();

#ifndef Q_OS_MAC
    QFont nameFont = ui->albumName->font();
    nameFont.setPointSize(nameFont.pointSize() - 4);
    ui->albumName->setFont(nameFont);
#endif

    QFont font = ui->labelCover->font();
#ifndef Q_OS_MAC
    font.setPointSize(font.pointSize() - 1);
#else
    font.setPointSize(font.pointSize() - 2);
#endif
    ui->labelCover->setFont(font);
    ui->labelDiscArt->setFont(font);

    ui->cover->setDefaultPixmap(QPixmap(":/img/placeholders/poster.png"));
    ui->discArt->setDefaultPixmap(QPixmap(":/img/placeholders/cd_art.png"));

    ui->genreCloud->setText(tr("Genres"));
    ui->genreCloud->setPlaceholder(tr("Add Genre"));
    connect(ui->genreCloud, &TagCloud::activated, this, &MusicWidgetAlbum::onAddCloudItem);
    connect(ui->genreCloud, &TagCloud::deactivated, this, &MusicWidgetAlbum::onRemoveCloudItem);

    ui->moodCloud->setText(tr("Moods"));
    ui->moodCloud->setPlaceholder(tr("Add Mood"));
    connect(ui->moodCloud, &TagCloud::activated, this, &MusicWidgetAlbum::onAddCloudItem);
    connect(ui->moodCloud, &TagCloud::deactivated, this, &MusicWidgetAlbum::onRemoveCloudItem);

    ui->styleCloud->setText(tr("Styles"));
    ui->styleCloud->setPlaceholder(tr("Add Style"));
    connect(ui->styleCloud, &TagCloud::activated, this, &MusicWidgetAlbum::onAddCloudItem);
    connect(ui->styleCloud, &TagCloud::deactivated, this, &MusicWidgetAlbum::onRemoveCloudItem);

    ui->cover->setImageType(ImageType::AlbumThumb);
    ui->discArt->setImageType(ImageType::AlbumCdArt);
    for (ClosableImage* image : ui->groupBox_3->findChildren<ClosableImage*>()) {
        connect(image, &ClosableImage::clicked, this, &MusicWidgetAlbum::onChooseImage);
        connect(image, &ClosableImage::sigClose, this, &MusicWidgetAlbum::onDeleteImage);
        connect(image, &ClosableImage::sigImageDropped, this, &MusicWidgetAlbum::onImageDropped);
    }

    m_bookletWidget = nullptr;

#ifdef Q_OS_WIN
    if (QGLFormat::openGLVersionFlags() & QGLFormat::OpenGL_Version_1_5) {
        m_bookletWidget = new ImageWidget(this);
        ui->verticalLayout_2->insertWidget(0, m_bookletWidget, 1);
        connect(m_bookletWidget, &ImageWidget::sigImageDropped, this, &MusicWidgetAlbum::onBookletsDropped);
        connect(ui->btnAddExtraFanart, &QPushButton::clicked, this, &MusicWidgetAlbum::onAddBooklet);
    } else {
        ui->tabWidget->removeTab(2);
    }
#else
    m_bookletWidget = new ImageWidget(this);
    ui->verticalLayout_2->insertWidget(0, m_bookletWidget, 1);
    connect(m_bookletWidget, &ImageWidget::sigImageDropped, this, &MusicWidgetAlbum::onBookletsDropped);
    connect(ui->btnAddExtraFanart, &QAbstractButton::clicked, this, &MusicWidgetAlbum::onAddBooklet);
#endif

    connect(ui->title, &QLineEdit::textChanged, ui->albumName, &QLabel::setText);
    connect(ui->buttonRevert, &QAbstractButton::clicked, this, &MusicWidgetAlbum::onRevertChanges);

    onSetEnabled(false);
    onClear();

    // clang-format off
    connect(ui->title,                     &QLineEdit::textEdited,       this, &MusicWidgetAlbum::onItemChanged);
    connect(ui->artist,                    &QLineEdit::textEdited,       this, &MusicWidgetAlbum::onItemChanged);
    connect(ui->label,                     &QLineEdit::textEdited,       this, &MusicWidgetAlbum::onItemChanged);
    connect(ui->releaseDate,               &QLineEdit::textEdited,       this, &MusicWidgetAlbum::onItemChanged);
    connect(ui->review,                    &QTextEdit::textChanged,      this, &MusicWidgetAlbum::onReviewChanged);
    connect(ui->musicBrainzAlbumId,        &QLineEdit::textEdited,       this, &MusicWidgetAlbum::onItemChanged);
    connect(ui->musicBrainzReleaseGroupId, &QLineEdit::textEdited,       this, &MusicWidgetAlbum::onItemChanged);
    connect(ui->year,   elchOverload<int>(&QSpinBox::valueChanged),          this, &MusicWidgetAlbum::onYearChanged);
    connect(ui->rating, elchOverload<double>(&QDoubleSpinBox::valueChanged), this, &MusicWidgetAlbum::onRatingChanged);
    // clang-format on

    QPainter p;
    QPixmap revert(":/img/arrow_circle_left.png");
    p.begin(&revert);
    p.setCompositionMode(QPainter::CompositionMode_SourceIn);
    p.fillRect(revert.rect(), QColor(0, 0, 0, 200));
    p.end();
    ui->buttonRevert->setIcon(QIcon(revert));
    ui->buttonRevert->setVisible(false);

    helper::applyStyle(ui->tabWidget);
    helper::applyStyle(ui->artWidget);
    helper::applyEffect(ui->groupBox_3);
}

MusicWidgetAlbum::~MusicWidgetAlbum()
{
    delete ui;
}

void MusicWidgetAlbum::setAlbum(Album* album)
{
    m_album = album;
    updateAlbumInfo();

    // clang-format off
    connect(m_album->controller(),   &AlbumController::sigInfoLoadDone,             this, &MusicWidgetAlbum::onInfoLoadDone,          Qt::UniqueConnection);
    connect(m_album->controller(),   &AlbumController::sigLoadDone,                 this, &MusicWidgetAlbum::onLoadDone,              Qt::UniqueConnection);
    connect(m_album->controller(),   &AlbumController::sigDownloadProgress,         this, &MusicWidgetAlbum::onDownloadProgress,      Qt::UniqueConnection);
    connect(m_album->controller(),   &AlbumController::sigLoadingImages,            this, &MusicWidgetAlbum::onLoadingImages,         Qt::UniqueConnection);
    connect(m_album->controller(),   &AlbumController::sigLoadImagesStarted,        this, &MusicWidgetAlbum::onLoadImagesStarted,     Qt::UniqueConnection);
    connect(m_album->controller(),   &AlbumController::sigImage,                    this, &MusicWidgetAlbum::onSetImage,              Qt::UniqueConnection);
    connect(m_album->bookletModel(), &ImageModel::hasChangedChanged,                this, &MusicWidgetAlbum::onBookletModelChanged,   Qt::UniqueConnection);
    // clang-format on

    onSetEnabled(!album->controller()->downloadsInProgress());
}

void MusicWidgetAlbum::onSetEnabled(bool enabled)
{
    if (m_album == nullptr) {
        ui->groupBox_3->setEnabled(false);
        return;
    }
    enabled = enabled && !m_album->controller()->downloadsInProgress();
    ui->groupBox_3->setEnabled(enabled);
    emit sigSetActionSearchEnabled(enabled, MainWidgets::Music);
    emit sigSetActionSaveEnabled(enabled, MainWidgets::Music);
}

void MusicWidgetAlbum::onClear()
{
    ui->albumName->clear();
    ui->path->clear();
    ui->path->setToolTip("");

    clearContents(ui->title);
    clearContents(ui->artist);
    clearContents(ui->label);
    clearContents(ui->releaseDate);
    clearContents(ui->musicBrainzAlbumId);
    clearContents(ui->musicBrainzReleaseGroupId);
    bool blocked = ui->review->blockSignals(true);
    ui->review->clear();
    ui->review->blockSignals(blocked);
    blocked = ui->rating->blockSignals(true);
    ui->rating->clear();
    ui->rating->blockSignals(blocked);
    blocked = ui->year->blockSignals(true);
    ui->year->clear();
    ui->year->blockSignals(blocked);

    ui->genreCloud->clear();
    ui->styleCloud->clear();
    ui->moodCloud->clear();
    ui->cover->clear();
    ui->discArt->clear();

    if (m_bookletWidget != nullptr) {
        m_bookletWidget->setAlbum(nullptr);
    }

    ui->buttonRevert->setVisible(false);
}

void MusicWidgetAlbum::clearContents(QLineEdit* widget)
{
    bool blocked = widget->blockSignals(true);
    widget->clear();
    widget->blockSignals(blocked);
}

void MusicWidgetAlbum::setContent(QLineEdit* widget, const QString& content)
{
    widget->blockSignals(true);
    widget->setText(content);
    widget->blockSignals(false);
}

void MusicWidgetAlbum::onSaveInformation()
{
    onSetEnabled(false);
    int id = NotificationBox::instance()->showMessage(tr("Saving Album..."));
    m_album->controller()->saveData(Manager::instance()->mediaCenterInterface());
    m_album->controller()->loadData(Manager::instance()->mediaCenterInterface(), true);
    updateAlbumInfo();
    onSetEnabled(true);
    NotificationBox::instance()->removeMessage(id);
    NotificationBox::instance()->showSuccess(tr("<b>\"%1\"</b> Saved").arg(m_album->title()));
    ui->buttonRevert->setVisible(false);
}

void MusicWidgetAlbum::onStartScraperSearch()
{
    if (m_album == nullptr) {
        return;
    }

    emit sigSetActionSearchEnabled(false, MainWidgets::Music);
    emit sigSetActionSaveEnabled(false, MainWidgets::Music);

    auto* searchWidget = new MusicSearch(this);
    searchWidget->execWithSearch("album",
        m_album->title(),
        (m_album->artist().isEmpty() && (m_album->artistObj() != nullptr)) ? m_album->artistObj()->name()
                                                                           : m_album->artist());

    if (searchWidget->result() == QDialog::Accepted) {
        onSetEnabled(false);
        m_album->controller()->loadData(MusicBrainzId(searchWidget->scraperId()),
            MusicBrainzId(searchWidget->scraperId2()),
            Manager::instance()->scrapers().musicScrapers().at(searchWidget->scraperNo()),
            searchWidget->infosToLoad());
        searchWidget->deleteLater();

    } else {
        searchWidget->deleteLater();
        emit sigSetActionSearchEnabled(true, MainWidgets::Music);
        emit sigSetActionSaveEnabled(true, MainWidgets::Music);
    }
}

void MusicWidgetAlbum::updateAlbumInfo()
{
    onClear();
    if (m_album == nullptr) {
        return;
    }

    ui->albumName->setText(m_album->title());
    setContent(ui->path, m_album->path().toNativePathString());
    ui->path->setToolTip(m_album->path().toNativePathString());
    setContent(ui->title, m_album->title());
    setContent(ui->artist, m_album->artist());
    setContent(ui->label, m_album->label());
    setContent(ui->releaseDate, m_album->releaseDate());
    setContent(ui->musicBrainzAlbumId, m_album->mbAlbumId().toString());
    setContent(ui->musicBrainzReleaseGroupId, m_album->mbReleaseGroupId().toString());
    ui->review->blockSignals(true);
    ui->review->setPlainText(m_album->review());
    ui->review->blockSignals(false);
    ui->year->blockSignals(true);
    ui->year->setValue(m_album->year());
    ui->year->blockSignals(false);
    ui->rating->blockSignals(true);
    ui->rating->setValue(m_album->rating());
    ui->rating->blockSignals(false);

    updateImage(ImageType::AlbumCdArt, ui->discArt);
    updateImage(ImageType::AlbumThumb, ui->cover);

    QStringList genres;
    QStringList styles;
    QStringList moods;
    for (const Artist* artist : Manager::instance()->musicModel()->artists()) {
        genres << artist->genres();
        styles << artist->styles();
        moods << artist->moods();
        for (int i = 0, n = artist->modelItem()->childNumber(); i < n; ++i) {
            if ((artist->modelItem()->child(i) != nullptr) && (artist->modelItem()->child(i)->album() != nullptr)) {
                genres << artist->modelItem()->child(i)->album()->genres();
                styles << artist->modelItem()->child(i)->album()->styles();
                moods << artist->modelItem()->child(i)->album()->moods();
            }
        }
    }

    // `setTags` requires distinct lists
    genres.removeDuplicates();
    styles.removeDuplicates();
    moods.removeDuplicates();

    ui->genreCloud->setTags(genres, m_album->genres());
    ui->styleCloud->setTags(styles, m_album->styles());
    ui->moodCloud->setTags(moods, m_album->moods());

    m_album->loadBooklets(Manager::instance()->mediaCenterInterface());
    if (m_bookletWidget != nullptr) {
        m_bookletWidget->setAlbum(m_album);
    }
}

void MusicWidgetAlbum::updateImage(ImageType imageType, ClosableImage* image)
{
    if (!m_album->rawImage(imageType).isNull()) {
        image->setImage(m_album->rawImage(imageType));
    } else if (!m_album->imagesToRemove().contains(imageType)) {
        QString imgFileName = Manager::instance()->mediaCenterInterface()->imageFileName(m_album, imageType);
        if (!imgFileName.isEmpty()) {
            image->setImage(imgFileName);
        }
    }
}

void MusicWidgetAlbum::onItemChanged(QString text)
{
    if (m_album == nullptr) {
        return;
    }

    auto* lineEdit = dynamic_cast<QLineEdit*>(sender());
    if (lineEdit == nullptr) {
        return;
    }

    QString property = lineEdit->property("item").toString();
    if (property == "artist") {
        m_album->setArtist(text);
    } else if (property == "title") {
        m_album->setTitle(text);
    } else if (property == "label") {
        m_album->setLabel(text);
    } else if (property == "releaseDate") {
        m_album->setReleaseDate(text);
    } else if (property == "mbAlbumId") {
        m_album->setMbAlbumId(MusicBrainzId(text));
    } else if (property == "mbReleaseGroupId") {
        m_album->setMbReleaseGroupId(MusicBrainzId(text));
    }

    ui->buttonRevert->setVisible(true);
}

void MusicWidgetAlbum::onReviewChanged()
{
    if (m_album == nullptr) {
        return;
    }
    m_album->setReview(ui->review->toPlainText());
    ui->buttonRevert->setVisible(true);
}

void MusicWidgetAlbum::onYearChanged(int value)
{
    if (m_album == nullptr) {
        return;
    }
    m_album->setYear(value);
    ui->buttonRevert->setVisible(true);
}

void MusicWidgetAlbum::onRatingChanged(double value)
{
    if (m_album == nullptr) {
        return;
    }
    m_album->setRating(value);
    ui->buttonRevert->setVisible(true);
}

void MusicWidgetAlbum::onRevertChanges()
{
    if (m_album == nullptr) {
        return;
    }

    m_album->clearImages();
    m_album->bookletModel()->clear();
    m_album->controller()->loadData(Manager::instance()->mediaCenterInterface(), true);
    updateAlbumInfo();
}

void MusicWidgetAlbum::onAddCloudItem(QString text)
{
    if (m_album == nullptr) {
        return;
    }

    QString property = sender()->property("item").toString();
    if (property == "genre") {
        m_album->addGenre(text);
    } else if (property == "style") {
        m_album->addStyle(text);
    } else if (property == "mood") {
        m_album->addMood(text);
    }

    ui->buttonRevert->setVisible(true);
}

void MusicWidgetAlbum::onRemoveCloudItem(QString text)
{
    if (m_album == nullptr) {
        return;
    }

    QString property = sender()->property("item").toString();
    if (property == "genre") {
        m_album->removeGenre(text);
    } else if (property == "style") {
        m_album->removeStyle(text);
    } else if (property == "mood") {
        m_album->removeMood(text);
    }

    ui->buttonRevert->setVisible(true);
}

void MusicWidgetAlbum::onChooseImage()
{
    if (m_album == nullptr) {
        return;
    }

    auto* image = dynamic_cast<ClosableImage*>(QObject::sender());
    if (image == nullptr) {
        return;
    }

    auto* imageDialog = new ImageDialog(this);
    imageDialog->setImageType(image->imageType());
    imageDialog->setAlbum(m_album);

    if (!m_album->images(image->imageType()).isEmpty()) {
        imageDialog->setDefaultDownloads(m_album->images(image->imageType()));
    }

    imageDialog->execWithType(image->imageType());
    const int exitCode = imageDialog->result();
    const QUrl imageUrl = imageDialog->imageUrl();
    imageDialog->deleteLater();

    if (exitCode == QDialog::Accepted) {
        emit sigSetActionSaveEnabled(false, MainWidgets::Music);
        m_album->controller()->loadImage(image->imageType(), imageUrl);
        ui->buttonRevert->setVisible(true);
    }
}

void MusicWidgetAlbum::onDeleteImage()
{
    if (m_album == nullptr) {
        return;
    }

    auto* image = dynamic_cast<ClosableImage*>(QObject::sender());
    if (image == nullptr) {
        return;
    }

    m_album->removeImage(image->imageType());
    updateImage(image->imageType(), image);
    ui->buttonRevert->setVisible(true);
}

void MusicWidgetAlbum::onImageDropped(ImageType imageType, QUrl imageUrl)
{
    if (m_album == nullptr) {
        return;
    }
    emit sigSetActionSaveEnabled(false, MainWidgets::Music);
    m_album->controller()->loadImage(imageType, imageUrl);
    ui->buttonRevert->setVisible(true);
}

void MusicWidgetAlbum::onInfoLoadDone(Album* album)
{
    if (m_album != album) {
        return;
    }

    updateAlbumInfo();
    ui->buttonRevert->setVisible(true);
    emit sigSetActionSaveEnabled(false, MainWidgets::Music);
}

void MusicWidgetAlbum::onLoadDone(Album* album)
{
    emit sigDownloadsFinished(Constants::MusicAlbumProgressMessageId + album->databaseId());

    if (m_album != album) {
        return;
    }

    if (m_bookletWidget != nullptr) {
        m_bookletWidget->setLoading(false);
    }
    onSetEnabled(true);
}

void MusicWidgetAlbum::onDownloadProgress(Album* album, int current, int maximum)
{
    emit sigDownloadsProgress(maximum - current, maximum, Constants::MusicAlbumProgressMessageId + album->databaseId());
}

void MusicWidgetAlbum::onLoadingImages(Album* album, QVector<ImageType> imageTypes)
{
    if (m_album != album) {
        return;
    }

    for (const auto imageType : imageTypes) {
        for (ClosableImage* cImage : ui->groupBox_3->findChildren<ClosableImage*>()) {
            if (cImage->imageType() == imageType) {
                cImage->setLoading(true);
            }
        }
    }

    if ((m_bookletWidget != nullptr) && imageTypes.contains(ImageType::AlbumBooklet)) {
        m_bookletWidget->setLoading(true);
    }

    ui->groupBox_3->update();
}

void MusicWidgetAlbum::onLoadImagesStarted(Album* album)
{
    emit sigDownloadsStarted(tr("Downloading images..."), Constants::MusicAlbumProgressMessageId + album->databaseId());
}

void MusicWidgetAlbum::onSetImage(Album* album, ImageType type, QByteArray imageData)
{
    if (m_album != album) {
        return;
    }

    for (ClosableImage* image : ui->groupBox_3->findChildren<ClosableImage*>()) {
        if (image->imageType() == type) {
            image->setLoading(false);
            image->setImage(imageData);
        }
    }
}

void MusicWidgetAlbum::onBookletModelChanged()
{
    auto* model = dynamic_cast<ImageModel*>(sender());
    if (model == nullptr) {
        return;
    }
    if (m_album != dynamic_cast<Album*>(model->parent())) {
        return;
    }
    if (model->hasChanged()) {
        ui->buttonRevert->setVisible(true);
        m_album->setHasChanged(true);
    }
}

void MusicWidgetAlbum::onAddBooklet()
{
    if (m_album == nullptr) {
        return;
    }

    auto* imageDialog = new ImageDialog(this);
    imageDialog->setImageType(ImageType::AlbumBooklet);
    imageDialog->setMultiSelection(true);
    imageDialog->setAlbum(m_album);

    imageDialog->execWithType(ImageType::AlbumBooklet);
    const int exitCode = imageDialog->result();
    const QVector<QUrl> imageUrls = imageDialog->imageUrls();
    imageDialog->deleteLater();

    if (exitCode == QDialog::Accepted && !imageUrls.isEmpty()) {
        if (m_bookletWidget != nullptr) {
            m_bookletWidget->setLoading(true);
        }
        emit sigSetActionSaveEnabled(false, MainWidgets::Music);
        m_album->controller()->loadImages(ImageType::AlbumBooklet, imageUrls);
        ui->buttonRevert->setVisible(true);
    }
}

void MusicWidgetAlbum::onBookletsDropped(QVector<QUrl> urls)
{
    if (m_bookletWidget != nullptr) {
        m_bookletWidget->setLoading(true);
    }
    m_album->controller()->loadImages(ImageType::AlbumBooklet, urls);
}
