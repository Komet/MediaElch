#include "MusicWidgetAlbum.h"
#include "ui_MusicWidgetAlbum.h"

#include <QGLFormat>
#include <QPainter>
#include "../globals/Globals.h"
#include "../globals/Helper.h"
#include "../globals/ImageDialog.h"
#include "../globals/Manager.h"
#include "../notifications/NotificationBox.h"
#include "MusicSearch.h"

MusicWidgetAlbum::MusicWidgetAlbum(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MusicWidgetAlbum)
{
    ui->setupUi(this);

    m_album = 0;
    ui->albumName->clear();

#ifndef Q_OS_MAC
    QFont nameFont = ui->albumName->font();
    nameFont.setPointSize(nameFont.pointSize()-4);
    ui->albumName->setFont(nameFont);
#endif

    QFont font = ui->labelCover->font();
    #ifndef Q_OS_MAC
        font.setPointSize(font.pointSize()-1);
    #else
        font.setPointSize(font.pointSize()-2);
    #endif
    ui->labelCover->setFont(font);
    ui->labelDiscArt->setFont(font);

    ui->cover->setDefaultPixmap(QPixmap(":/img/placeholders/poster.png"));
    ui->discArt->setDefaultPixmap(QPixmap(":/img/placeholders/cd_art.png"));

    ui->genreCloud->setText(tr("Genres"));
    ui->genreCloud->setPlaceholder(tr("Add Genre"));
    connect(ui->genreCloud, SIGNAL(activated(QString)), this, SLOT(onAddCloudItem(QString)));
    connect(ui->genreCloud, SIGNAL(deactivated(QString)), this, SLOT(onRemoveCloudItem(QString)));

    ui->moodCloud->setText(tr("Moods"));
    ui->moodCloud->setPlaceholder(tr("Add Mood"));
    connect(ui->moodCloud, SIGNAL(activated(QString)), this, SLOT(onAddCloudItem(QString)));
    connect(ui->moodCloud, SIGNAL(deactivated(QString)), this, SLOT(onRemoveCloudItem(QString)));

    ui->styleCloud->setText(tr("Styles"));
    ui->styleCloud->setPlaceholder(tr("Add Style"));
    connect(ui->styleCloud, SIGNAL(activated(QString)), this, SLOT(onAddCloudItem(QString)));
    connect(ui->styleCloud, SIGNAL(deactivated(QString)), this, SLOT(onRemoveCloudItem(QString)));

    ui->cover->setImageType(ImageType::AlbumThumb);
    ui->discArt->setImageType(ImageType::AlbumCdArt);
    foreach (ClosableImage *image, ui->groupBox_3->findChildren<ClosableImage*>()) {
        connect(image, &ClosableImage::clicked, this, &MusicWidgetAlbum::onChooseImage);
        connect(image, &ClosableImage::sigClose, this, &MusicWidgetAlbum::onDeleteImage);
        connect(image, &ClosableImage::sigImageDropped, this, &MusicWidgetAlbum::onImageDropped);
    }

    m_bookletWidget = 0;

    if (QGLFormat::openGLVersionFlags() & QGLFormat::OpenGL_Version_1_5) {
        m_bookletWidget = new ImageWidget(this);
        ui->verticalLayout_2->insertWidget(0, m_bookletWidget, 1);
        connect(m_bookletWidget, &ImageWidget::sigImageDropped, this, &MusicWidgetAlbum::onBookletsDropped);
        connect(ui->btnAddExtraFanart, SIGNAL(clicked()), this, SLOT(onAddBooklet()));
    } else {
        ui->tabWidget->removeTab(2);
    }

    connect(ui->title, SIGNAL(textChanged(QString)), ui->albumName, SLOT(setText(QString)));
    connect(ui->buttonRevert, SIGNAL(clicked()), this, SLOT(onRevertChanges()));

    onSetEnabled(false);
    onClear();

    connect(ui->title, SIGNAL(textEdited(QString)), this, SLOT(onItemChanged(QString)));
    connect(ui->artist, SIGNAL(textEdited(QString)), this, SLOT(onItemChanged(QString)));
    connect(ui->label, SIGNAL(textEdited(QString)), this, SLOT(onItemChanged(QString)));
    connect(ui->releaseDate, SIGNAL(textEdited(QString)), this, SLOT(onItemChanged(QString)));
    connect(ui->year, SIGNAL(valueChanged(int)), this, SLOT(onYearChanged(int)));
    connect(ui->rating, SIGNAL(valueChanged(double)), this, SLOT(onRatingChanged(double)));
    connect(ui->review, SIGNAL(textChanged()), this, SLOT(onReviewChanged()));
    connect(ui->musicBrainzAlbumId, SIGNAL(textEdited(QString)), this, SLOT(onItemChanged(QString)));
    connect(ui->musicBrainzReleaseGroupId, SIGNAL(textEdited(QString)), this, SLOT(onItemChanged(QString)));

    QPainter p;
    QPixmap revert(":/img/arrow_circle_left.png");
    p.begin(&revert);
    p.setCompositionMode(QPainter::CompositionMode_SourceIn);
    p.fillRect(revert.rect(), QColor(0, 0, 0, 200));
    p.end();
    ui->buttonRevert->setIcon(QIcon(revert));
    ui->buttonRevert->setVisible(false);

    Helper::instance()->applyStyle(ui->tabWidget);
    Helper::instance()->applyStyle(ui->artWidget);
    Helper::instance()->applyEffect(ui->groupBox_3);
}

MusicWidgetAlbum::~MusicWidgetAlbum()
{
    delete ui;
}

void MusicWidgetAlbum::setAlbum(Album *album)
{
    m_album = album;
    updateAlbumInfo();

    connect(m_album->controller(), SIGNAL(sigInfoLoadDone(Album*)), this, SLOT(onInfoLoadDone(Album*)), Qt::UniqueConnection);
    connect(m_album->controller(), SIGNAL(sigLoadDone(Album*)), this, SLOT(onLoadDone(Album*)), Qt::UniqueConnection);
    connect(m_album->controller(), SIGNAL(sigDownloadProgress(Album*,int, int)), this, SLOT(onDownloadProgress(Album*,int,int)), Qt::UniqueConnection);
    connect(m_album->controller(), SIGNAL(sigLoadingImages(Album*,QList<int>)), this, SLOT(onLoadingImages(Album*,QList<int>)), Qt::UniqueConnection);
    connect(m_album->controller(), SIGNAL(sigLoadImagesStarted(Album*)), this, SLOT(onLoadImagesStarted(Album*)), Qt::UniqueConnection);
    connect(m_album->controller(), SIGNAL(sigImage(Album*,int,QByteArray)), this, SLOT(onSetImage(Album*,int,QByteArray)), Qt::UniqueConnection);
    connect(m_album->bookletModel(), SIGNAL(hasChangedChanged()), this, SLOT(onBookletModelChanged()), Qt::UniqueConnection);

    onSetEnabled(!album->controller()->downloadsInProgress());
}

void MusicWidgetAlbum::onSetEnabled(bool enabled)
{
    if (!m_album) {
        ui->groupBox_3->setEnabled(false);
        return;
    }
    enabled = enabled && !m_album->controller()->downloadsInProgress();
    ui->groupBox_3->setEnabled(enabled);
    emit sigSetActionSearchEnabled(enabled, WidgetMusic);
    emit sigSetActionSaveEnabled(enabled, WidgetMusic);
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

    if (m_bookletWidget)
        m_bookletWidget->setAlbum(0);

    ui->buttonRevert->setVisible(false);
}

void MusicWidgetAlbum::clearContents(QLineEdit *widget)
{
    bool blocked = widget->blockSignals(true);
    widget->clear();
    widget->blockSignals(blocked);
}

void MusicWidgetAlbum::setContent(QLineEdit *widget, const QString &content)
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
    NotificationBox::instance()->showMessage(tr("<b>\"%1\"</b> Saved").arg(m_album->title()));
    ui->buttonRevert->setVisible(false);
}

void MusicWidgetAlbum::onStartScraperSearch()
{
    if (!m_album)
        return;

    emit sigSetActionSearchEnabled(false, WidgetMusic);
    emit sigSetActionSaveEnabled(false, WidgetMusic);

    MusicSearch::instance()->exec("album", m_album->title(), (m_album->artist().isEmpty() && m_album->artistObj()) ? m_album->artistObj()->name() : m_album->artist());

    if (MusicSearch::instance()->result() == QDialog::Accepted) {
        onSetEnabled(false);
        m_album->controller()->loadData(MusicSearch::instance()->scraperId(),
                                        MusicSearch::instance()->scraperId2(),
                                        Manager::instance()->musicScrapers().at(MusicSearch::instance()->scraperNo()),
                                        MusicSearch::instance()->infosToLoad());
    } else {
        emit sigSetActionSearchEnabled(true, WidgetMusic);
        emit sigSetActionSaveEnabled(true, WidgetMusic);
    }
}

void MusicWidgetAlbum::updateAlbumInfo()
{
    onClear();
    if (!m_album)
        return;

    ui->albumName->setText(m_album->title());
    setContent(ui->path, m_album->path());
    ui->path->setToolTip(m_album->path());
    setContent(ui->title, m_album->title());
    setContent(ui->artist, m_album->artist());
    setContent(ui->label, m_album->label());
    setContent(ui->releaseDate, m_album->releaseDate());
    setContent(ui->musicBrainzAlbumId, m_album->mbAlbumId());
    setContent(ui->musicBrainzReleaseGroupId, m_album->mbReleaseGroupId());
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
    foreach (Artist *artist, Manager::instance()->musicModel()->artists()) {
        genres << artist->genres();
        styles << artist->styles();
        moods << artist->moods();
        for (int i=0, n=artist->modelItem()->childNumber() ; i<n ; ++i) {
            if (artist->modelItem()->child(i) && artist->modelItem()->child(i)->album()) {
                genres << artist->modelItem()->child(i)->album()->genres();
                styles << artist->modelItem()->child(i)->album()->styles();
                moods << artist->modelItem()->child(i)->album()->moods();
            }
        }
    }
    genres.removeDuplicates();
    styles.removeDuplicates();
    moods.removeDuplicates();
    ui->genreCloud->setTags(genres, m_album->genres());
    ui->styleCloud->setTags(styles, m_album->styles());
    ui->moodCloud->setTags(moods, m_album->moods());

    m_album->loadBooklets(Manager::instance()->mediaCenterInterface());
    if (m_bookletWidget)
        m_bookletWidget->setAlbum(m_album);
}

void MusicWidgetAlbum::updateImage(int imageType, ClosableImage *image)
{
    if (!m_album->rawImage(imageType).isNull()) {
        image->setImage(m_album->rawImage(imageType));
    } else if (!m_album->imagesToRemove().contains(imageType)) {
        QString imgFileName = Manager::instance()->mediaCenterInterface()->imageFileName(m_album, imageType);
        if (!imgFileName.isEmpty())
            image->setImage(imgFileName);
    }
}

void MusicWidgetAlbum::onItemChanged(QString text)
{
    if (!m_album)
        return;

    QLineEdit *lineEdit = static_cast<QLineEdit*>(sender());
    if (!lineEdit)
        return;

    QString property = lineEdit->property("item").toString();
    if (property == "artist")
        m_album->setArtist(text);
    else if (property == "title")
        m_album->setTitle(text);
    else if (property == "label")
        m_album->setLabel(text);
    else if (property == "releaseDate")
        m_album->setReleaseDate(text);
    else if (property == "mbAlbumId")
        m_album->setMbAlbumId(text);
    else if (property == "mbReleaseGroupId")
        m_album->setMbReleaseGroupId(text);

    ui->buttonRevert->setVisible(true);
}

void MusicWidgetAlbum::onReviewChanged()
{
    if (!m_album)
        return;
    m_album->setReview(ui->review->toPlainText());
    ui->buttonRevert->setVisible(true);
}

void MusicWidgetAlbum::onYearChanged(int value)
{
    if (!m_album)
        return;
    m_album->setYear(value);
    ui->buttonRevert->setVisible(true);
}

void MusicWidgetAlbum::onRatingChanged(double value)
{
    if (!m_album)
        return;
    m_album->setRating(value);
    ui->buttonRevert->setVisible(true);
}

void MusicWidgetAlbum::onRevertChanges()
{
    if (!m_album)
        return;

    m_album->clearImages();
    m_album->bookletModel()->clear();
    m_album->controller()->loadData(Manager::instance()->mediaCenterInterface(), true);
    updateAlbumInfo();
}

void MusicWidgetAlbum::onAddCloudItem(QString text)
{
    if (!m_album)
        return;

    QString property = sender()->property("item").toString();
    if (property == "genre")
        m_album->addGenre(text);
    else if (property == "style")
        m_album->addStyle(text);
    else if (property == "mood")
        m_album->addMood(text);

    ui->buttonRevert->setVisible(true);
}

void MusicWidgetAlbum::onRemoveCloudItem(QString text)
{
    if (!m_album)
        return;

    QString property = sender()->property("item").toString();
    if (property == "genre")
        m_album->removeGenre(text);
    else if (property == "style")
        m_album->removeStyle(text);
    else if (property == "mood")
        m_album->removeMood(text);

    ui->buttonRevert->setVisible(true);
}

void MusicWidgetAlbum::onChooseImage()
{
    if (m_album == 0)
        return;

    ClosableImage *image = static_cast<ClosableImage*>(QObject::sender());
    if (!image)
        return;

    ImageDialog::instance()->setImageType(image->imageType());
    ImageDialog::instance()->clear();
    ImageDialog::instance()->setAlbum(m_album);

    if (!m_album->images(image->imageType()).isEmpty())
        ImageDialog::instance()->setDownloads(m_album->images(image->imageType()));
    else
        ImageDialog::instance()->setDownloads(QList<Poster>());

    ImageDialog::instance()->exec(image->imageType());

    if (ImageDialog::instance()->result() == QDialog::Accepted) {
        emit sigSetActionSaveEnabled(false, WidgetMusic);
        m_album->controller()->loadImage(image->imageType(), ImageDialog::instance()->imageUrl());
        ui->buttonRevert->setVisible(true);
    }
}

void MusicWidgetAlbum::onDeleteImage()
{
    if (m_album == 0)
        return;

    ClosableImage *image = static_cast<ClosableImage*>(QObject::sender());
    if (!image)
        return;

    m_album->removeImage(image->imageType());
    updateImage(image->imageType(), image);
    ui->buttonRevert->setVisible(true);
}

void MusicWidgetAlbum::onImageDropped(int imageType, QUrl imageUrl)
{
    if (!m_album)
        return;
    emit sigSetActionSaveEnabled(false, WidgetMusic);
    m_album->controller()->loadImage(imageType, imageUrl);
    ui->buttonRevert->setVisible(true);
}

void MusicWidgetAlbum::onInfoLoadDone(Album *album)
{
    if (m_album != album)
        return;

    updateAlbumInfo();
    ui->buttonRevert->setVisible(true);
    emit sigSetActionSaveEnabled(false, WidgetMusic);
}

void MusicWidgetAlbum::onLoadDone(Album *album)
{
    emit sigDownloadsFinished(Constants::MusicAlbumProgressMessageId+album->databaseId());

    if (m_album != album)
        return;

    if (m_bookletWidget)
        m_bookletWidget->setLoading(false);
    onSetEnabled(true);
}

void MusicWidgetAlbum::onDownloadProgress(Album *album, int current, int maximum)
{
    emit sigDownloadsProgress(maximum-current, maximum, Constants::MusicAlbumProgressMessageId+album->databaseId());
}

void MusicWidgetAlbum::onLoadingImages(Album *album, QList<int> imageTypes)
{
    if (m_album != album)
        return;

    foreach (const int &imageType, imageTypes) {
        foreach (ClosableImage *cImage, ui->groupBox_3->findChildren<ClosableImage*>()) {
            if (cImage->imageType() == imageType)
                cImage->setLoading(true);
        }
    }

    if (m_bookletWidget && imageTypes.contains(ImageType::AlbumBooklet))
        m_bookletWidget->setLoading(true);

    ui->groupBox_3->update();
}

void MusicWidgetAlbum::onLoadImagesStarted(Album *album)
{
    emit sigDownloadsStarted(tr("Downloading images..."), Constants::MusicAlbumProgressMessageId+album->databaseId());
}

void MusicWidgetAlbum::onSetImage(Album *album, int type, QByteArray data)
{
    if (m_album != album)
        return;

    foreach (ClosableImage *image, ui->groupBox_3->findChildren<ClosableImage*>()) {
        if (image->imageType() == type) {
            image->setLoading(false);
            image->setImage(data);
        }
    }
}

void MusicWidgetAlbum::onBookletModelChanged()
{
    ImageModel *model = static_cast<ImageModel*>(sender());
    if (!model)
        return;
    if (m_album != static_cast<Album*>(model->parent()))
        return;
    if (model->hasChanged()) {
        ui->buttonRevert->setVisible(true);
        m_album->setHasChanged(true);
    }
}

void MusicWidgetAlbum::onAddBooklet()
{
    if (!m_album)
        return;

    ImageDialog::instance()->setImageType(ImageType::AlbumBooklet);
    ImageDialog::instance()->clear();
    ImageDialog::instance()->setMultiSelection(true);
    ImageDialog::instance()->setAlbum(m_album);
    ImageDialog::instance()->setDownloads(QList<Poster>());
    ImageDialog::instance()->exec(ImageType::AlbumBooklet);

    if (ImageDialog::instance()->result() == QDialog::Accepted && !ImageDialog::instance()->imageUrls().isEmpty()) {
        if (m_bookletWidget)
            m_bookletWidget->setLoading(true);
        emit sigSetActionSaveEnabled(false, WidgetMusic);
        m_album->controller()->loadImages(ImageType::AlbumBooklet, ImageDialog::instance()->imageUrls());
        ui->buttonRevert->setVisible(true);
    }
}

void MusicWidgetAlbum::onBookletsDropped(QList<QUrl> urls)
{
    if (m_bookletWidget)
        m_bookletWidget->setLoading(true);
    m_album->controller()->loadImages(ImageType::AlbumBooklet, urls);
}
