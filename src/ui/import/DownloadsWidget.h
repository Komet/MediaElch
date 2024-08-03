#pragma once

#include "import/DownloadFileSearcher.h"
#include "import/Extractor.h"
#include "ui/import/MakeMkvDialog.h"

#include <QComboBox>
#include <QElapsedTimer>
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
    void onChangeImportType(int currentIndex);
    void onChangeImportType(int currentIndex, QComboBox* box);
    void onChangeImportDetail(int currentIndex);
    void onChangeImportDetail(int currentIndex, QComboBox* box);
    void onImportWithMakeMkv();

    void onScanFinished(mediaelch::DownloadFileSearcher* searcher);

private:
    Ui::DownloadsWidget* ui;
    ImportSettings* m_importSettings{nullptr};

    QMap<QString, mediaelch::DownloadFileSearcher::Package> m_packages;
    QMap<QString, mediaelch::DownloadFileSearcher::Import> m_imports;
    Extractor* m_extractor;

    QMutex m_mutex;
    QElapsedTimer m_scanTimer;
    bool m_isSearchInProgress = false;

    MakeMkvDialog* m_makeMkvDialog;
};
