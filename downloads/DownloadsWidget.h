#ifndef DOWNLOADSWIDGET_H
#define DOWNLOADSWIDGET_H

#include <QComboBox>
#include <QFileInfo>
#include <QMap>
#include <QMutex>
#include <QWidget>

#include "downloads/Extractor.h"
#include "downloads/MakeMkvDialog.h"

namespace Ui {
class DownloadsWidget;
}

class DownloadsWidget : public QWidget
{
    Q_OBJECT

    struct Package
    {
        QString baseName;
        QStringList files;
        qreal size;
    };

    struct Import
    {
        QString baseName;
        QStringList files;
        QStringList extraFiles;
        qreal size;
    };

public:
    explicit DownloadsWidget(QWidget *parent = nullptr);
    ~DownloadsWidget();
    void updatePackagesList(QMap<QString, Package> packages);
    void updateImportsList(QMap<QString, Import> imports);
    int hasNewItems();

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
    void onChangeImportType(int currentIndex, QComboBox *sender = nullptr);
    void onChangeImportDetail(int currentIndex, QComboBox *sender = nullptr);
    void onImportWithMakeMkv();

private:
    Ui::DownloadsWidget *ui;

    QString baseName(QFileInfo fileInfo) const;
    bool isPackage(QFileInfo file) const;
    bool isImportable(QFileInfo file) const;
    bool isSubtitle(QFileInfo file) const;

    QMap<QString, Package> m_packages;
    QMap<QString, Import> m_imports;
    Extractor *m_extractor;
    QMutex m_mutex;
    MakeMkvDialog *m_makeMkvDialog;
};

#endif // DOWNLOADSWIDGET_H
