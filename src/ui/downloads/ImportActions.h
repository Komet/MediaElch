#pragma once

#include "tv_shows/TvShow.h"
#include "ui/downloads/ImportDialog.h"

#include <QWidget>

namespace Ui {
class ImportActions;
}

class ImportActions : public QWidget
{
    Q_OBJECT

public:
    explicit ImportActions(QWidget* parent = nullptr);
    ~ImportActions() override;
    void setButtonEnabled(bool enabled);
    void setBaseName(QString baseName);
    QString baseName();
    void setType(QString type);
    QString type();
    void setTvShow(TvShow* show);
    TvShow* tvShow();
    void setImportDir(QString dir);
    QString importDir();
    void setFiles(QStringList files);
    QStringList files();
    void setExtraFiles(QStringList extraFiles);
    QStringList extraFiles();

signals:
    void sigDelete(QString);
    void sigDialogClosed();

private slots:
    void onImport();
    void onDelete();

private:
    Ui::ImportActions* ui;
    QString m_baseName;
    QString m_type;
    QString m_importDir;
    TvShow* m_tvShow;
    ImportDialog* m_importDialog;
    QStringList m_files;
    QStringList m_extraFiles;
};
