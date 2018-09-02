#ifndef MUSICWIDGETARTIST_H
#define MUSICWIDGETARTIST_H

#include <QLineEdit>
#include <QPointer>
#include <QTableWidgetItem>
#include <QWidget>

#include "../music/Artist.h"
#include "../smallWidgets/ClosableImage.h"

namespace Ui {
class MusicWidgetArtist;
}

class MusicWidgetArtist : public QWidget
{
    Q_OBJECT

public:
    explicit MusicWidgetArtist(QWidget *parent = nullptr);
    ~MusicWidgetArtist() override;
    void setArtist(Artist *artist);
    void updateArtistInfo();

public slots:
    void onSetEnabled(bool enabled);
    void onClear();
    void onSaveInformation();
    void onStartScraperSearch();

signals:
    void sigSetActionSearchEnabled(bool, MainWidgets);
    void sigSetActionSaveEnabled(bool, MainWidgets);
    void sigDownloadsStarted(QString, int);
    void sigDownloadsProgress(int, int, int);
    void sigDownloadsFinished(int);

private slots:
    void onRevertChanges();
    void onItemChanged(QString text);
    void onBiographyChanged();
    void onAddCloudItem(QString text);
    void onRemoveCloudItem(QString text);
    void onChooseImage();
    void onDeleteImage();
    void onImageDropped(ImageType imageType, QUrl imageUrl);
    void onInfoLoadDone(Artist *artist);
    void onLoadDone(Artist *artist);
    void onDownloadProgress(Artist *artist, int current, int maximum);
    void onLoadingImages(Artist *artist, QList<ImageType> imageTypes);
    void onLoadImagesStarted(Artist *artist);
    void onSetImage(Artist *artist, ImageType type, QByteArray data);
    void onRemoveExtraFanart(const QString &file);
    void onRemoveExtraFanart(const QByteArray &image);
    void onAddExtraFanart();
    void onExtraFanartDropped(QUrl imageUrl);
    void onAddAlbum();
    void onRemoveAlbum();
    void onAlbumEdited(QTableWidgetItem *item);

private:
    Ui::MusicWidgetArtist *ui;
    QPointer<Artist> m_artist;

    void clearContents(QLineEdit *widget);
    void setContent(QLineEdit *widget, const QString &content);
    void updateImage(ImageType imageType, ClosableImage *image);
};

#endif // MUSICWIDGETARTIST_H
