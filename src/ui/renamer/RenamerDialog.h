#pragma once

#include "renamer/PlaceholderParser.h"
#include "renamer/Renamer.h"

#include <QDialog>
#include <QMap>
#include <QString>
#include <QStringList>

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
    bool m_filesRenamed{false};
    mediaelch::FileFilter m_extraFiles;
    bool m_renameErrorOccurred{false};
};
