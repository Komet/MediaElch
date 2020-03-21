#pragma once

#include "downloads/Extractor.h"
#include "ui/downloads/MakeMkvDialog.h"

#include <QComboBox>
#include <QFileInfo>
#include <QMap>
#include <QMutex>
#include <QWidget>

namespace Ui {
class DownloadsWidget;
}

class DownloadsWidget : public QWidget
{
    Q_OBJECT

public:
    struct Package
    {
        QString baseName;
        QStringList files;
        /// Size in Bytes of this package.
        /// Not an int to allow sizes >4GB on 32bit systems
        double size;
    };

    struct Import
    {
        QString baseName;
        QStringList files;
        QStringList extraFiles;
        /// Size in Bytes of this import.
        /// Not an int to allow sizes >4GB on 32bit systems
        double size;
    };

public:
    explicit DownloadsWidget(QWidget* parent = nullptr);
    ~DownloadsWidget() override;
    void updatePackagesList(const QMap<QString, Package>& packages);
    void updateImportsList(const QMap<QString, Import>& imports);
    int hasNewItems();

public slots:
    void scanDownloadsAndImports();
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
    void onChangeImportType(int currentIndex, QComboBox* sender = nullptr);
    void onChangeImportDetail(int currentIndex, QComboBox* sender = nullptr);
    void onImportWithMakeMkv();

private:
    Ui::DownloadsWidget* ui;

    QMap<QString, Package> m_packages;
    QMap<QString, Import> m_imports;
    Extractor* m_extractor;
    QMutex m_mutex;
    MakeMkvDialog* m_makeMkvDialog;
};
