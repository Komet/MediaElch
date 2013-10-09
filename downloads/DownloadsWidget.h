#ifndef DOWNLOADSWIDGET_H
#define DOWNLOADSWIDGET_H

#include <QComboBox>
#include <QFileInfo>
#include <QFileSystemWatcher>
#include <QMap>
#include <QMutex>
#include <QWidget>
#include "downloads/Extractor.h"

namespace Ui {
class DownloadsWidget;
}

class DownloadsWidget : public QWidget
{
    Q_OBJECT

    struct Package {
        QString baseName;
        QStringList files;
        qreal size;
    };

    struct Import {
        QString baseName;
        QStringList files;
        QStringList extraFiles;
        qreal size;
    };

public:
    explicit DownloadsWidget(QWidget *parent = 0);
    ~DownloadsWidget();
    void updatePackagesList(QMap<QString, Package> packages);
    void updateImportsList(QMap<QString, Import> imports);
    bool hasNewItems();

public slots:
    void scanDownloadFolders(bool scanDownloads = true, bool scanImports = true);

signals:
    void sigScanFinished(bool);

private slots:
    void onUnpack(QString baseName, QString password);
    void onDelete(QString baseName);
    void onDeleteImport(QString baseName);
    void onExtractorError(QString baseName, QString msg);
    void onExtractorFinished(QString baseName, bool success);
    void onExtractorProgress(QString baseName, int progress);
    void onChangeImportType(int currentIndex, QComboBox *sender = 0);
    void onChangeImportDetail(int currentIndex, QComboBox *sender = 0);

private:
    Ui::DownloadsWidget *ui;

    QString baseName(QFileInfo fileInfo) const;
    bool isPackage(QFileInfo file) const;
    bool isImportable(QFileInfo file) const;
    bool isSubtitle(QFileInfo file) const;

    QMap<QString, Package> m_packages;
    QMap<QString, Import> m_imports;
    Extractor *m_extractor;
    QFileSystemWatcher *m_watcher;
    QMutex m_mutex;
};

#endif // DOWNLOADSWIDGET_H
