#include "MusicWidgetArtist.h"
#include "ui_MusicWidgetArtist.h"

#include <QPainter>
#include "../globals/Globals.h"
#include "../globals/Helper.h"
#include "../globals/ImageDialog.h"
#include "../globals/Manager.h"
#include "../notifications/NotificationBox.h"
#include "MusicSearch.h"

MusicWidgetArtist::MusicWidgetArtist(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MusicWidgetArtist)
{
    ui->setupUi(this);

    m_artist = 0;

    ui->artistName->clear();
#ifndef Q_OS_MAC
    QFont nameFont = ui->artistName->font();
    nameFont.setPointSize(nameFont.pointSize()-4);
    ui->artistName->setFont(nameFont);
#endif

    QFont font = ui->labelFanart->font();
    #ifndef Q_OS_MAC
        font.setPointSize(font.pointSize()-1);
    #else
        font.setPointSize(font.pointSize()-2);
    #endif
    ui->labelFanart->setFont(font);
    ui->labelLogo->setFont(font);
    ui->labelThumb->setFont(font);

    ui->fanart->setDefaultPixmap(QPixmap(":/img/placeholders/fanart.png"));
    ui->logo->setDefaultPixmap(QPixmap(":/img/placeholders/logo.png"));
    ui->thumb->setDefaultPixmap(QPixmap(":/img/placeholders/thumb.png"));

    ui->discography->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui->discography->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->btnAddAlbum->setIcon(Manager::instance()->iconFont()->icon("plus", QColor(255, 255, 255)));
    ui->btnRemoveAlbum->setIcon(Manager::instance()->iconFont()->icon("close", QColor(255, 255, 255)));

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

    ui->logo->setImageType(ImageType::ArtistLogo);
    ui->fanart->setImageType(ImageType::ArtistFanart);
    ui->thumb->setImageType(ImageType::ArtistThumb);
    foreach (ClosableImage *image, ui->groupBox_3->findChildren<ClosableImage*>()) {
        connect(image, &ClosableImage::clicked, this, &MusicWidgetArtist::onChooseImage);
        connect(image, &ClosableImage::sigClose, this, &MusicWidgetArtist::onDeleteImage);
        connect(image, &ClosableImage::sigImageDropped, this, &MusicWidgetArtist::onImageDropped);
    }

    connect(ui->name, SIGNAL(textChanged(QString)), ui->artistName, SLOT(setText(QString)));
    connect(ui->buttonRevert, SIGNAL(clicked()), this, SLOT(onRevertChanges()));

    onSetEnabled(false);
    onClear();

    connect(ui->name, SIGNAL(textEdited(QString)), this, SLOT(onItemChanged(QString)));
    connect(ui->born, SIGNAL(textEdited(QString)), this, SLOT(onItemChanged(QString)));
    connect(ui->formed, SIGNAL(textEdited(QString)), this, SLOT(onItemChanged(QString)));
    connect(ui->yearsActive, SIGNAL(textEdited(QString)), this, SLOT(onItemChanged(QString)));
    connect(ui->disbanded, SIGNAL(textEdited(QString)), this, SLOT(onItemChanged(QString)));
    connect(ui->died, SIGNAL(textEdited(QString)), this, SLOT(onItemChanged(QString)));
    connect(ui->biography, SIGNAL(textChanged()), this, SLOT(onBiographyChanged()));
    connect(ui->musicBrainzId, SIGNAL(textEdited(QString)), this, SLOT(onItemChanged(QString)));

    connect(ui->fanarts, SIGNAL(sigRemoveImage(QByteArray)), this, SLOT(onRemoveExtraFanart(QByteArray)));
    connect(ui->fanarts, SIGNAL(sigRemoveImage(QString)), this, SLOT(onRemoveExtraFanart(QString)));
    connect(ui->btnAddExtraFanart, SIGNAL(clicked()), this, SLOT(onAddExtraFanart()));
    connect(ui->fanarts, &ImageGallery::sigImageDropped, this, &MusicWidgetArtist::onExtraFanartDropped);

    connect(ui->btnAddAlbum, SIGNAL(clicked()), this, SLOT(onAddAlbum()));
    connect(ui->btnRemoveAlbum, SIGNAL(clicked()), this, SLOT(onRemoveAlbum()));
    connect(ui->discography, SIGNAL(itemChanged(QTableWidgetItem*)), this, SLOT(onAlbumEdited(QTableWidgetItem*)));

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

MusicWidgetArtist::~MusicWidgetArtist()
{
    delete ui;
}

void MusicWidgetArtist::setArtist(Artist *artist)
{
    m_artist = artist;
    updateArtistInfo();

    connect(m_artist->controller(), SIGNAL(sigInfoLoadDone(Artist*)), this, SLOT(onInfoLoadDone(Artist*)), Qt::UniqueConnection);
    connect(m_artist->controller(), SIGNAL(sigLoadDone(Artist*)), this, SLOT(onLoadDone(Artist*)), Qt::UniqueConnection);
    connect(m_artist->controller(), SIGNAL(sigDownloadProgress(Artist*,int, int)), this, SLOT(onDownloadProgress(Artist*,int,int)), Qt::UniqueConnection);
    connect(m_artist->controller(), SIGNAL(sigLoadingImages(Artist*,QList<int>)), this, SLOT(onLoadingImages(Artist*,QList<int>)), Qt::UniqueConnection);
    connect(m_artist->controller(), SIGNAL(sigLoadImagesStarted(Artist*)), this, SLOT(onLoadImagesStarted(Artist*)), Qt::UniqueConnection);
    connect(m_artist->controller(), SIGNAL(sigImage(Artist*,int,QByteArray)), this, SLOT(onSetImage(Artist*,int,QByteArray)), Qt::UniqueConnection);

    onSetEnabled(!artist->controller()->downloadsInProgress());
}

void MusicWidgetArtist::onSetEnabled(bool enabled)
{
    if (!m_artist) {
        ui->groupBox_3->setEnabled(false);
        return;
    }
    enabled = enabled && !m_artist->controller()->downloadsInProgress();
    ui->groupBox_3->setEnabled(enabled);
    emit sigSetActionSearchEnabled(enabled, WidgetMusic);
    emit sigSetActionSaveEnabled(enabled, WidgetMusic);
}

void MusicWidgetArtist::onClear()
{
    ui->artistName->clear();
    ui->path->clear();
    ui->path->setToolTip("");

    clearContents(ui->name);
    clearContents(ui->born);
    clearContents(ui->formed);
    clearContents(ui->yearsActive);
    clearContents(ui->disbanded);
    clearContents(ui->died);
    clearContents(ui->musicBrainzId);

    bool blocked = ui->biography->blockSignals(true);
    ui->biography->clear();
    ui->biography->blockSignals(blocked);

    blocked = ui->discography->blockSignals(true);
    ui->discography->setRowCount(0);
    ui->discography->blockSignals(false);

    ui->genreCloud->clear();
    ui->styleCloud->clear();
    ui->moodCloud->clear();
    ui->thumb->clear();
    ui->fanart->clear();
    ui->logo->clear();
    ui->fanarts->clear();

    ui->buttonRevert->setVisible(false);
}

void MusicWidgetArtist::clearContents(QLineEdit *widget)
{
    bool blocked = widget->blockSignals(true);
    widget->clear();
    widget->blockSignals(blocked);
}

void MusicWidgetArtist::setContent(QLineEdit *widget, const QString &content)
{
    widget->blockSignals(true);
    widget->setText(content);
    widget->blockSignals(false);
}

void MusicWidgetArtist::onSaveInformation()
{
    onSetEnabled(false);
    int id = NotificationBox::instance()->showMessage(tr("Saving Artist..."));
    m_artist->controller()->saveData(Manager::instance()->mediaCenterInterface());
    m_artist->controller()->loadData(Manager::instance()->mediaCenterInterface(), true);
    updateArtistInfo();
    onSetEnabled(true);
    NotificationBox::instance()->removeMessage(id);
    NotificationBox::instance()->showMessage(tr("<b>\"%1\"</b> Saved").arg(m_artist->name()));
    ui->buttonRevert->setVisible(false);
}

void MusicWidgetArtist::onStartScraperSearch()
{
    if (!m_artist)
        return;

    emit sigSetActionSearchEnabled(false, WidgetMusic);
    emit sigSetActionSaveEnabled(false, WidgetMusic);

    MusicSearch::instance()->exec("artist", m_artist->name());

    if (MusicSearch::instance()->result() == QDialog::Accepted) {
        onSetEnabled(false);
        m_artist->controller()->loadData(MusicSearch::instance()->scraperId(),
                                         Manager::instance()->musicScrapers().at(MusicSearch::instance()->scraperNo()),
                                         MusicSearch::instance()->infosToLoad());
    } else {
        emit sigSetActionSearchEnabled(true, WidgetMusic);
        emit sigSetActionSaveEnabled(true, WidgetMusic);
    }
}

void MusicWidgetArtist::updateArtistInfo()
{
    onClear();
    if (!m_artist)
        return;

    ui->artistName->setText(m_artist->name());
    setContent(ui->path, m_artist->path());
    ui->path->setToolTip(m_artist->path());
    setContent(ui->name, m_artist->name());
    setContent(ui->born, m_artist->born());
    setContent(ui->formed, m_artist->formed());
    setContent(ui->yearsActive, m_artist->yearsActive());
    setContent(ui->disbanded, m_artist->disbanded());
    setContent(ui->died, m_artist->died());
    setContent(ui->musicBrainzId, m_artist->mbId());
    ui->biography->blockSignals(true);
    ui->biography->setPlainText(m_artist->biography());
    ui->biography->blockSignals(false);

    ui->discography->blockSignals(true);
    foreach (DiscographyAlbum *album, m_artist->discographyAlbumsPointer()) {
        int row = ui->discography->rowCount();
        ui->discography->insertRow(row);
        ui->discography->setItem(row, 0, new QTableWidgetItem(album->title));
        ui->discography->setItem(row, 1, new QTableWidgetItem(album->year));
        ui->discography->item(row, 0)->setData(Qt::UserRole, QVariant::fromValue(album));
    }
    ui->discography->blockSignals(false);

    updateImage(ImageType::ArtistFanart, ui->fanart);
    updateImage(ImageType::ArtistThumb, ui->thumb);
    updateImage(ImageType::ArtistLogo, ui->logo);
    ui->fanarts->setImages(m_artist->extraFanarts(Manager::instance()->mediaCenterInterface()));

    QStringList genres;
    QStringList styles;
    QStringList moods;
    foreach (Artist *artist, Manager::instance()->musicModel()->artists()) {
        genres << artist->genres();
        styles << artist->styles();
        moods << artist->moods();
    }
    genres.removeDuplicates();
    styles.removeDuplicates();
    moods.removeDuplicates();
    ui->genreCloud->setTags(genres, m_artist->genres());
    ui->styleCloud->setTags(styles, m_artist->styles());
    ui->moodCloud->setTags(moods, m_artist->moods());
}

void MusicWidgetArtist::updateImage(int imageType, ClosableImage *image)
{
    if (!m_artist->rawImage(imageType).isNull()) {
        image->setImage(m_artist->rawImage(imageType));
    } else if (!m_artist->imagesToRemove().contains(imageType)) {
        QString imgFileName = Manager::instance()->mediaCenterInterface()->imageFileName(m_artist, imageType);
        if (!imgFileName.isEmpty())
            image->setImage(imgFileName);
    }
}

void MusicWidgetArtist::onItemChanged(QString text)
{
    if (!m_artist)
        return;

    QLineEdit *lineEdit = static_cast<QLineEdit*>(sender());
    if (!lineEdit)
        return;

    QString property = lineEdit->property("item").toString();
    if (property == "name")
        m_artist->setName(text);
    else if (property == "born")
        m_artist->setBorn(text);
    else if (property == "formed")
        m_artist->setFormed(text);
    else if (property == "yearsActive")
        m_artist->setYearsActive(text);
    else if (property == "disbanded")
        m_artist->setDisbanded(text);
    else if (property == "died")
        m_artist->setDied(text);
    else if (property == "mbid")
        m_artist->setMbId(text);

    ui->buttonRevert->setVisible(true);
}

void MusicWidgetArtist::onBiographyChanged()
{
    if (!m_artist)
        return;
    m_artist->setBiography(ui->biography->toPlainText());
    ui->buttonRevert->setVisible(true);
}

void MusicWidgetArtist::onRevertChanges()
{
    if (!m_artist)
        return;

    m_artist->clearImages();
    m_artist->controller()->loadData(Manager::instance()->mediaCenterInterface(), true);
    updateArtistInfo();
}

void MusicWidgetArtist::onAddCloudItem(QString text)
{
    if (!m_artist)
        return;

    QString property = sender()->property("item").toString();
    if (property == "genre")
        m_artist->addGenre(text);
    else if (property == "style")
        m_artist->addStyle(text);
    else if (property == "mood")
        m_artist->addMood(text);

    ui->buttonRevert->setVisible(true);
}

void MusicWidgetArtist::onRemoveCloudItem(QString text)
{
    if (!m_artist)
        return;

    QString property = sender()->property("item").toString();
    if (property == "genre")
        m_artist->removeGenre(text);
    else if (property == "style")
        m_artist->removeStyle(text);
    else if (property == "mood")
        m_artist->removeMood(text);

    ui->buttonRevert->setVisible(true);
}

void MusicWidgetArtist::onChooseImage()
{
    if (m_artist == 0)
        return;

    ClosableImage *image = static_cast<ClosableImage*>(QObject::sender());
    if (!image)
        return;

    ImageDialog::instance()->setImageType(image->imageType());
    ImageDialog::instance()->clear();
    ImageDialog::instance()->setArtist(m_artist);

    if (!m_artist->images(image->imageType()).isEmpty())
        ImageDialog::instance()->setDownloads(m_artist->images(image->imageType()));
    else
        ImageDialog::instance()->setDownloads(QList<Poster>());

    ImageDialog::instance()->exec(image->imageType());

    if (ImageDialog::instance()->result() == QDialog::Accepted) {
        emit sigSetActionSaveEnabled(false, WidgetMusic);
        m_artist->controller()->loadImage(image->imageType(), ImageDialog::instance()->imageUrl());
        ui->buttonRevert->setVisible(true);
    }
}

void MusicWidgetArtist::onDeleteImage()
{
    if (m_artist == 0)
        return;

    ClosableImage *image = static_cast<ClosableImage*>(QObject::sender());
    if (!image)
        return;

    m_artist->removeImage(image->imageType());
    updateImage(image->imageType(), image);
    ui->buttonRevert->setVisible(true);
}

void MusicWidgetArtist::onImageDropped(int imageType, QUrl imageUrl)
{
    if (!m_artist)
        return;
    emit sigSetActionSaveEnabled(false, WidgetMusic);
    m_artist->controller()->loadImage(imageType, imageUrl);
    ui->buttonRevert->setVisible(true);
}

void MusicWidgetArtist::onInfoLoadDone(Artist *artist)
{
    if (m_artist != artist)
        return;

    updateArtistInfo();
    ui->buttonRevert->setVisible(true);
    emit sigSetActionSaveEnabled(false, WidgetMusic);
}

void MusicWidgetArtist::onLoadDone(Artist *artist)
{
    emit sigDownloadsFinished(Constants::MusicArtistProgressMessageId+artist->databaseId());

    if (m_artist != artist)
        return;

    ui->fanarts->setLoading(false);
    onSetEnabled(true);
}

void MusicWidgetArtist::onDownloadProgress(Artist *artist, int current, int maximum)
{
    emit sigDownloadsProgress(maximum-current, maximum, Constants::MusicArtistProgressMessageId+artist->databaseId());
}

void MusicWidgetArtist::onLoadingImages(Artist *artist, QList<int> imageTypes)
{
    if (m_artist != artist)
        return;

    foreach (const int &imageType, imageTypes) {
        foreach (ClosableImage *cImage, ui->groupBox_3->findChildren<ClosableImage*>()) {
            if (cImage->imageType() == imageType)
                cImage->setLoading(true);
        }
    }

    if (imageTypes.contains(ImageType::ArtistExtraFanart))
        ui->fanarts->setLoading(true);

    ui->groupBox_3->update();
}

void MusicWidgetArtist::onLoadImagesStarted(Artist *artist)
{
    emit sigDownloadsStarted(tr("Downloading images..."), Constants::MusicArtistProgressMessageId+artist->databaseId());
}

void MusicWidgetArtist::onSetImage(Artist *artist, int type, QByteArray data)
{
    if (artist != m_artist)
        return;

    if (type == ImageType::ArtistExtraFanart) {
        ui->fanarts->addImage(data);
        return;
    }

    foreach (ClosableImage *image, ui->groupBox_3->findChildren<ClosableImage*>()) {
        if (image->imageType() == type) {
            image->setLoading(false);
            image->setImage(data);
        }
    }
}

void MusicWidgetArtist::onRemoveExtraFanart(const QByteArray &image)
{
    if (!m_artist)
        return;
    m_artist->removeExtraFanart(image);
    ui->buttonRevert->setVisible(true);
}

void MusicWidgetArtist::onRemoveExtraFanart(const QString &file)
{
    if (!m_artist)
        return;
    m_artist->removeExtraFanart(file);
    ui->buttonRevert->setVisible(true);
}

void MusicWidgetArtist::onAddExtraFanart()
{
    if (!m_artist)
        return;

    ImageDialog::instance()->setImageType(ImageType::ArtistExtraFanart);
    ImageDialog::instance()->clear();
    ImageDialog::instance()->setMultiSelection(true);
    ImageDialog::instance()->setArtist(m_artist);
    ImageDialog::instance()->setDownloads(m_artist->images(ImageType::ArtistFanart));
    ImageDialog::instance()->exec(ImageType::ArtistFanart);

    if (ImageDialog::instance()->result() == QDialog::Accepted && !ImageDialog::instance()->imageUrls().isEmpty()) {
        ui->fanarts->setLoading(true);
        emit sigSetActionSaveEnabled(false, WidgetMusic);
        m_artist->controller()->loadImages(ImageType::ArtistExtraFanart, ImageDialog::instance()->imageUrls());
        ui->buttonRevert->setVisible(true);
    }
}

void MusicWidgetArtist::onExtraFanartDropped(QUrl imageUrl)
{
    if (!m_artist)
        return;
    ui->fanarts->setLoading(true);
    emit sigSetActionSaveEnabled(false, WidgetMusic);
    m_artist->controller()->loadImages(ImageType::ArtistExtraFanart, QList<QUrl>() << imageUrl);
    ui->buttonRevert->setVisible(true);
}

void MusicWidgetArtist::onAddAlbum()
{
    if (!m_artist)
        return;

    DiscographyAlbum a;
    a.title = tr("Unknown Album");
    a.year = "";
    m_artist->addDiscographyAlbum(a);

    DiscographyAlbum *album = m_artist->discographyAlbumsPointer().last();

    ui->discography->blockSignals(true);
    int row = ui->discography->rowCount();
    ui->discography->insertRow(row);
    ui->discography->setItem(row, 0, new QTableWidgetItem(album->title));
    ui->discography->setItem(row, 1, new QTableWidgetItem(album->year));
    ui->discography->item(row, 0)->setData(Qt::UserRole, QVariant::fromValue(album));
    ui->discography->scrollToBottom();
    ui->discography->blockSignals(false);
    ui->buttonRevert->setVisible(true);
}

void MusicWidgetArtist::onRemoveAlbum()
{
    int row = ui->discography->currentRow();
    if (!m_artist || row < 0 || row >= ui->discography->rowCount() || !ui->discography->currentItem()->isSelected())
        return;

    DiscographyAlbum *album = ui->discography->item(row, 0)->data(Qt::UserRole).value<DiscographyAlbum*>();
    if (!album)
        return;
    m_artist->removeDiscographyAlbum(album);
    ui->discography->blockSignals(true);
    ui->discography->removeRow(row);
    ui->discography->blockSignals(false);
    ui->buttonRevert->setVisible(true);
}

void MusicWidgetArtist::onAlbumEdited(QTableWidgetItem *item)
{
    DiscographyAlbum *album = ui->discography->item(item->row(), 0)->data(Qt::UserRole).value<DiscographyAlbum*>();
    if (item->column() == 0)
        album->title = item->text();
    else if (item->column() == 1)
        album->year = item->text();
    m_artist->setHasChanged(true);
    ui->buttonRevert->setVisible(true);
}
