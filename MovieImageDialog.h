#ifndef MOVIEIMAGEDIALOG_H
#define MOVIEIMAGEDIALOG_H

#include <QDialog>
#include <QLabel>
#include <QUrl>
#include <QWidget>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QResizeEvent>

#include "Globals.h"

namespace Ui {
class MovieImageDialog;
}

class MovieImageDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit MovieImageDialog(QWidget *parent = 0);
    ~MovieImageDialog();
    static MovieImageDialog *instance(QWidget *parent = 0);
    void setDownloads(QList<Poster> downloads);
    QUrl imageUrl();
    void setImageType(ImageType type);
    void clear();
    void cancelDownloads();

public slots:
    void accept();
    void reject();

protected:
    void resizeEvent(QResizeEvent *event);

private slots:
    void downloadFinished();
    void startNextDownload();
    void imageClicked(int row, int col);

private:
    Ui::MovieImageDialog *ui;

    struct DownloadElement {
        QUrl thumbUrl;
        QUrl originalUrl;
        QPixmap pixmap;
        bool downloaded;
        QLabel *cellWidget;
    };

    QNetworkAccessManager m_qnam;
    int m_currentDownloadIndex;
    QNetworkReply *m_currentDownloadReply;
    ImageType m_imageType;
    QList<DownloadElement> m_elements;
    QUrl m_imageUrl;

    QNetworkAccessManager *qnam();
    void renderTable();
    int calcColumnCount();
    int getColumnWidth();
};

#endif // MOVIEIMAGEDIALOG_H
