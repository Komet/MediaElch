#ifndef IMAGEGALLERY_H
#define IMAGEGALLERY_H

#include <QResizeEvent>
#include <QScrollArea>
#include <QToolButton>
#include <QWidget>
#include "globals/Globals.h"
#include "smallWidgets/ClosableImage.h"

class ImageGallery : public QWidget
{
    Q_OBJECT
public:
    explicit ImageGallery(QWidget *parent = 0);
    void clear();
    void setImages(QList<ExtraFanart> images);
    void addImage(const QByteArray &img, const QString &url = QString());
    void setLoading(const bool &loading);
    void setAlignment(const int &alignment);
    void setShowZoomAndResolution(const bool &show);

signals:
    void sigRemoveImage(QByteArray);
    void sigRemoveImage(QString);
    void sigImageDropped(QUrl);

protected:
    void resizeEvent(QResizeEvent *event);

private slots:
    void onCloseImage();
    void onVerticalScrollBarMoved(const int &value);
    void onHorizontalScrollBarMoved(const int &value);
    void onButtonLeft();
    void onButtonRight();
    void onButtonTop();
    void onButtonBottom();
    void onVerticalScrollBarRangeChanged(int min, int max);
    void onHorizontalScrollBarRangeChanged(int min, int max);
    void dragEnterEvent(QDragEnterEvent *event);
    void dragMoveEvent(QDragMoveEvent *event);
    void dropEvent(QDropEvent *event);

private:
    QLabel *m_loadingLabel;
    QList<ClosableImage*> m_imageLabels;
    QWidget *m_imagesWidget;
    QScrollArea *m_scrollArea;
    QToolButton *m_buttonLeft;
    QToolButton *m_buttonRight;
    QToolButton *m_buttonTop;
    QToolButton *m_buttonBottom;
    int m_imageWidth;
    int m_imageHeight;
    int m_horizontalSpace;
    int m_verticalSpace;
    int m_alignment;
    int m_scrollValue;
    bool m_showZoomAndResolution;

    void positionImages();
};

#endif // IMAGEGALLERY_H
