#ifndef IMAGEDIALOG_H
#define IMAGEDIALOG_H

#include <QDialog>
#include <QLabel>
#include <QUrl>
#include <QWidget>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QResizeEvent>

#include "globals/Globals.h"
#include "smallWidgets/ImageLabel.h"

namespace Ui {
class ImageDialog;
}

/**
 * @brief The ImageDialog class
 * Displays a set of images
 */
class ImageDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ImageDialog(QWidget *parent = 0);
    ~ImageDialog();
    static ImageDialog *instance(QWidget *parent = 0);
    void setDownloads(QList<Poster> downloads);
    QUrl imageUrl();
    void setImageType(ImageType type);
    void clear();
    void cancelDownloads();

public slots:
    void accept();
    void reject();
    int exec(int type);

protected:
    void resizeEvent(QResizeEvent *event);

private slots:
    void downloadFinished();
    void startNextDownload();
    void imageClicked(int row, int col);
    void chooseLocalImage();
    void onImageDropped(QUrl url);
    void onPreviewSizeChange(int value);
    void onZoomIn();
    void onZoomOut();

private:
    Ui::ImageDialog *ui;

    /**
     * @brief The DownloadElement struct
     */
    struct DownloadElement {
        QUrl thumbUrl;
        QUrl originalUrl;
        QPixmap pixmap;
        QPixmap scaledPixmap;
        bool downloaded;
        ImageLabel *cellWidget;
        QSize resolution;
    };

    QNetworkAccessManager m_qnam;
    int m_currentDownloadIndex;
    QNetworkReply *m_currentDownloadReply;
    ImageType m_imageType;
    QList<DownloadElement> m_elements;
    QUrl m_imageUrl;
    int m_type;
    QLabel *m_noElementsLabel;

    QNetworkAccessManager *qnam();
    void renderTable();
    int calcColumnCount();
    int getColumnWidth();
};

#endif // IMAGEDIALOG_H
