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
    Ui::ExportDialog* ui = nullptr;
    mediaelch::MediaExport* m_exporter = nullptr;
    volatile bool m_canceled = false;
};
