#pragma once

#include "export/ExportTemplate.h"

#include <QWidget>

namespace Ui {
class ExportTemplateWidget;
}

class ExportTemplateWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ExportTemplateWidget(QWidget* parent = nullptr);
    ~ExportTemplateWidget() override;
    void setExportTemplate(ExportTemplate* exportTemplate);

private slots:
    void onBtnInstall();

private:
    Ui::ExportTemplateWidget* ui = nullptr;
    ExportTemplate* m_exportTemplate = nullptr;
};
