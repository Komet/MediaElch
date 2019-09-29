#pragma once

#include "export/ExportTemplate.h"
#include "export/MediaExport.h"

#include <QDialog>
#include <QDir>
#include <QString>
#include <QStringList>
#include <QVector>
#include <atomic>

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
    /// If somehow a non-existent theme is selected, let the user know and log the incident.
    void warnAboutInvalidTheme();
    /// Get the currently selected theme name (combo box)
    QString themeName() const;
    /// Get the currently selected sections / media types that should be exported.
    QVector<ExportTemplate::ExportSection> sectionsToExport() const;
    /// Get the number of items in the user's library. Only counts items if the corresponding section is given.
    int libraryItemCount(const QVector<ExportTemplate::ExportSection>& sections) const;

    Ui::ExportDialog* ui = nullptr;
    std::atomic_bool m_canceled{false};
    int m_itemsExported = 0;
};
