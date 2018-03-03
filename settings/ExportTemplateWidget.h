#ifndef EXPORTTEMPLATEWIDGET_H
#define EXPORTTEMPLATEWIDGET_H

#include <QWidget>

#include "export/ExportTemplate.h"

namespace Ui {
class ExportTemplateWidget;
}

class ExportTemplateWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ExportTemplateWidget(QWidget *parent = nullptr);
    ~ExportTemplateWidget() override;
    void setExportTemplate(ExportTemplate *exportTemplate);

private slots:
    void onBtnInstall();

private:
    Ui::ExportTemplateWidget *ui;
    ExportTemplate *m_exportTemplate;
};

#endif // EXPORTTEMPLATEWIDGET_H
