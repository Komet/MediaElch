#ifndef CONCERTWIDGET_H
#define CONCERTWIDGET_H

#include <QContextMenuEvent>
#include <QLabel>
#include <QMenu>
#include <QPointer>
#include <QResizeEvent>
#include <QTableWidgetItem>
#include <QWidget>

#include "data/Concert.h"
#include "smallWidgets/ClosableImage.h"

namespace Ui {
class ConcertWidget;
}

/**
 * @brief The ConcertWidget class
 */
class ConcertWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ConcertWidget(QWidget *parent = 0);
    ~ConcertWidget();

public slots:
    void clear();
    void setConcert(Concert *concert);
    void onStartScraperSearch();
    void onSaveInformation();
    void onSaveAll();
    void setEnabledTrue(Concert *concert = 0);
    void setDisabledTrue();
    void setBigWindow(bool bigWindow);
    void updateConcertInfo();

protected:
    void resizeEvent(QResizeEvent *event);

signals:
    void setActionSearchEnabled(bool, MainWidgets);
    void setActionSaveEnabled(bool, MainWidgets);

private slots:
    void onInfoLoadDone(Concert *concert);
    void onLoadDone(Concert *concert);
    void onLoadImagesStarted(Concert *concert);
    void onLoadingImages(Concert *concert, QList<int> imageTypes);
    void onDownloadProgress(Concert *concert, int current, int maximum);
    void onSetImage(Concert *concert, int type, QByteArray data);

    void onChooseImage();
    void onDeleteImage();
    void onImageDropped(int imageType, QUrl imageUrl);

    void concertNameChanged(QString text);
    void addGenre(QString genre);
    void removeGenre(QString genre);
    void addTag(QString tag);
    void removeTag(QString tag);
    void onRevertChanges();
    void onArtPageOne();
    void onArtPageTwo();

    void onNameChange(QString text);
    void onArtistChange(QString text);
    void onAlbumChange(QString text);
    void onTaglineChange(QString text);
    void onRatingChange(double value);
    void onReleasedChange(QDate date);
    void onRuntimeChange(int value);
    void onCertificationChange(QString text);
    void onTrailerChange(QString text);
    void onWatchedClicked();
    void onPlayCountChange(int value);
    void onLastWatchedChange(QDateTime dateTime);
    void onOverviewChange();

    void onStreamDetailsEdited();
    void onReloadStreamDetails();
    void updateStreamDetails(bool reloadFromFile = false);

    void onRemoveExtraFanart(const QString &file);
    void onRemoveExtraFanart(const QByteArray &image);
    void onAddExtraFanart();
    void onExtraFanartDropped(QUrl imageUrl);

    void updateImage(const int &imageType, ClosableImage *image);

private:
    Ui::ConcertWidget *ui;
    QPointer<Concert> m_concert;
    QMovie *m_loadingMovie;
    QLabel *m_savingWidget;
    QList<QWidget*> m_streamDetailsWidgets;
    QList< QList<QLineEdit*> > m_streamDetailsAudio;
    QList< QList<QLineEdit*> > m_streamDetailsSubtitles;
    void updateImages(QList<int> images);
};

#endif // CONCERTWIDGET_H
