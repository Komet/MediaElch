#pragma once

#include "imports/DownloadFileSearcher.h"
#include "imports/Extractor.h"
#include "ui/imports/MakeMkvDialog.h"

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
    explicit DownloadsWidget(QWidget* parent = nullptr);
    ~DownloadsWidget() override;
    void updatePackagesList(const QMap<QString, mediaelch::DownloadFileSearcher::Package>& packages);
    void updateImportsList(const QMap<QString, mediaelch::DownloadFileSearcher::Import>& imports);
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

    QMap<QString, mediaelch::DownloadFileSearcher::Package> m_packages;
    QMap<QString, mediaelch::DownloadFileSearcher::Import> m_imports;
    Extractor* m_extractor;
    QMutex m_mutex;
    MakeMkvDialog* m_makeMkvDialog;
};
