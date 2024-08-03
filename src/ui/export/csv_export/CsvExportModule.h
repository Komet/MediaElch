#pragma once

#include "globals/Module.h"
#include "ui/export/csv_export/CsvExportConfiguration.h"
#include "utils/Meta.h"

#include <QWidget>

class Settings;
class CsvExportDialog;

namespace mediaelch {
namespace exporter {


class CsvExportModule : public core::ModuleInterface
{
public:
    explicit CsvExportModule(Settings& settings);
    QString moduleName() override;
    ~CsvExportModule() override;

    void onInit() override;

public:
    ELCH_NODISCARD CsvExportDialog* makeCsvExportDialog(QWidget* parent = nullptr);

private:
    CsvExportConfiguration m_csvExportSettings;
};

} // namespace exporter
} // namespace mediaelch
