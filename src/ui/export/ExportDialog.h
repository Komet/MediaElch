#pragma once

#include <QDialog>
#include <QDir>
#include <QString>
#include <QStringList>
#include <QVector>

#include "export/ExportTemplate.h"
#include "export/MediaExport.h"

namespace Ui {
class ExportDialog;
}

class ExportDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ExportDialog(QWidget* parent = nullptr);
    ~ExportDialog() override;

public slots:
    int exec() override;

private slots:
    void onBtnExport();
    void onThemeChanged();
    void onBtnClose();

private:
    /// Resets the export progress, i.e. clears all messages and resets the progress bar
    /// Useful if the selected theme is changed by the user.
    void resetProgress();

    Ui::ExportDialog* ui = nullptr;
    mediaelch::MediaExport* m_exporter = nullptr;
    volatile bool m_canceled = false;
};
