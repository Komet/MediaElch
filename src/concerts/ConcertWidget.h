#ifndef CONCERTWIDGET_H
#define CONCERTWIDGET_H

#include "data/Concert.h"

#include <QContextMenuEvent>
#include <QLabel>
#include <QMenu>
#include <QPointer>
#include <QResizeEvent>
#include <QTableWidgetItem>
#include <QWidget>

namespace Ui {
class ConcertWidget;
}

class ClosableImage;

/**
 * @brief The ConcertWidget class
 */
class ConcertWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ConcertWidget(QWidget *parent = nullptr);
    ~ConcertWidget() override;

public slots:
    void clear();
    void setConcert(Concert *concert);
    void onStartScraperSearch();
    void onSaveInformation();
    void onSaveAll();
    void setEnabledTrue(Concert *concert = nullptr);
    void setDisabledTrue();
    void setBigWindow(bool bigWindow);
    void updateConcertInfo();

protected:
    void resizeEvent(QResizeEvent *event) override;

signals:
    void setActionSearchEnabled(bool, MainWidgets);
    void setActionSaveEnabled(bool, MainWidgets);

private slots:
    void onInfoLoadDone(Concert *concert);
    void onLoadDone(Concert *concert);
    void onLoadImagesStarted(Concert *concert);
    void onLoadingImages(Concert *concert, QList<ImageType> imageTypes);
    void onDownloadProgress(Concert *concert, int current, int maximum);
    void onSetImage(Concert *concert, ImageType type, QByteArray data);

    void onChooseImage();
    void onDeleteImage();
    void onImageDropped(ImageType imageType, QUrl imageUrl);

    void concertNameChanged(QString text);
    void addGenre(QString genre);
    void removeGenre(QString genre);
    void addTag(QString tag);
    void removeTag(QString tag);
    void onRevertChanges();
    void onArtPageOne();
    void onArtPageTwo();

    void onStreamDetailsEdited();
    void onReloadStreamDetails();
    void updateStreamDetails(bool reloadFromFile = false);

    void onRemoveExtraFanart(const QString &file);
    void onRemoveExtraFanart(const QByteArray &image);
    void onAddExtraFanart();
    void onExtraFanartDropped(QUrl imageUrl);

    void updateImage(ImageType imageType, ClosableImage *image);

private:
    Ui::ConcertWidget *ui;
    QPointer<Concert> m_concert = nullptr;
    QMovie *m_loadingMovie;
    QLabel *m_savingWidget;
    QList<QWidget *> m_streamDetailsWidgets;
    QList<QList<QLineEdit *>> m_streamDetailsAudio;
    QList<QList<QLineEdit *>> m_streamDetailsSubtitles;
    void updateImages(QList<ImageType> images);
};

#endif // CONCERTWIDGET_H
