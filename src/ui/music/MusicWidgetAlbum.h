#pragma once

#include "music/Album.h"
#include "ui/image/ImageWidget.h"
#include "ui/small_widgets/ClosableImage.h"

#include <QLineEdit>
#include <QPointer>
#include <QWidget>

namespace Ui {
class MusicWidgetAlbum;
}

class MusicWidgetAlbum : public QWidget
{
    Q_OBJECT

public:
    explicit MusicWidgetAlbum(QWidget* parent = nullptr);
    ~MusicWidgetAlbum() override;
    void setAlbum(Album* album);

public slots:
    void onSetEnabled(bool enabled);
    void onClear();
    void onSaveInformation();
    void onStartScraperSearch();
    void updateAlbumInfo();

signals:
    void sigSetActionSearchEnabled(bool, MainWidgets);
    void sigSetActionSaveEnabled(bool, MainWidgets);
    void sigDownloadsStarted(QString, int);
    void sigDownloadsProgress(int, int, int);
    void sigDownloadsFinished(int);

private slots:
    void onRevertChanges();
    void onItemChanged(QString text);
    void onRatingChanged(double value);
    void onYearChanged(int value);
    void onReviewChanged();
    void onAddCloudItem(QString text);
    void onRemoveCloudItem(QString text);
    void onChooseImage();
    void onDeleteImage();
    void onImageDropped(ImageType imageType, QUrl imageUrl);
    void onInfoLoadDone(Album* album);
    void onLoadDone(Album* album);
    void onDownloadProgress(Album* album, int current, int maximum);
    void onLoadingImages(Album* album, QVector<ImageType> imageTypes);
    void onLoadImagesStarted(Album* album);
    void onSetImage(Album* album, ImageType type, QByteArray imageData);
    void onBookletModelChanged();
    void onAddBooklet();
    void onBookletsDropped(QVector<QUrl> urls);

private:
    Ui::MusicWidgetAlbum* ui;
    QPointer<Album> m_album;
    ImageWidget* m_bookletWidget;

    void clearContents(QLineEdit* widget);
    void setContent(QLineEdit* widget, const QString& content);
    void updateImage(ImageType imageType, ClosableImage* image);
};
