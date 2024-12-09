#pragma once

#include "data/concert/Concert.h"
#include "data/movie/Movie.h"
#include "data/tv_show/TvShow.h"
#include "data/tv_show/TvShowEpisode.h"
#include "renamer/Renamer.h"

#include <QDialog>
#include <QDir>
#include <QFile>

namespace Ui {
class RenamerDialog;
}

class RenamerDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RenamerDialog(QWidget* parent = nullptr);
    ~RenamerDialog() override;

    ELCH_NODISCARD bool renameErrorOccurred() const;

    int addResultToTable(const QString& oldFileName, const QString& newFileName, Renamer::RenameOperation operation);
    void setResultStatus(int row, Renamer::RenameResult result);
    void appendResultText(QString str);

public slots:
    int exec() override;
    void reject() override;

signals:
    void sigFilesRenamed(RenameType type, bool hasErrors);

private slots:
    void onRename();
    void onDryRun();
    void onChkRenameDirectories();
    void onChkRenameFiles();
    void onChkUseSeasonDirectories();
    void onChkReplaceDelimiter();
    void onRenamed();

protected:
    virtual void renameType(bool isDryRun) = 0;
    virtual void rejectImpl() = 0;
    virtual QString dialogInfoLabel() = 0;
    virtual QStringList fileNameDefaults() = 0;
    virtual QStringList fileNameMultiDefaults() = 0;
    virtual QStringList directoryNameDefaults() = 0;

protected:
    Ui::RenamerDialog* ui = nullptr;

    RenameType m_renameType = RenameType::All;
    bool m_filesRenamed = 0;
    mediaelch::FileFilter m_extraFiles;
    bool m_renameErrorOccured = 0;
};
