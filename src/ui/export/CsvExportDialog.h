#pragma once

#include "export/CsvExport.h"

#include <QDialog>

namespace Ui {
class CsvExportDialog;
}

class CsvExportDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CsvExportDialog(QWidget *parent = nullptr);
    ~CsvExportDialog();

public slots:
    int exec() override;

private slots:
    void onExport();

private:
    void initializeItems();
    QVector<mediaelch::CsvMovieExport::MovieField> getMovieFields() const;
    QString defaultCsvFileName() const;

private:
    Ui::CsvExportDialog *ui;
};
