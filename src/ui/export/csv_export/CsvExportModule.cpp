#include "ui/export/csv_export/CsvExportModule.h"

#include "ui/export/csv_export/CsvExportDialog.h"

static QString MODULE_NAME = "csv_export";

namespace mediaelch {
namespace exporter {

CsvExportModule::CsvExportModule(Settings& settings) : m_csvExportSettings(settings)
{
}

QString CsvExportModule::moduleName()
{
    return MODULE_NAME;
}

CsvExportModule::~CsvExportModule()
{
}

void CsvExportModule::onInit()
{
    m_csvExportSettings.init();
}

CsvExportDialog* CsvExportModule::makeCsvExportDialog(QWidget* parent)
{
    return new CsvExportDialog(m_csvExportSettings, parent);
}


} // namespace exporter
} // namespace mediaelch
