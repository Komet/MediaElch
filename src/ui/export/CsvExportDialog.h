#pragma once

#include "export/CsvExport.h"

#include <QDialog>
#include <QDir>
#include <QListWidget>

namespace Ui {
class CsvExportDialog;
}

class CsvExportDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CsvExportDialog(QWidget* parent = nullptr);
    ~CsvExportDialog();

public slots:
    int exec() override;

private slots:
    void onExport();

private:
    void initializeItems();

    template<typename T>
    QVector<T> getFields(QListWidget* widget) const
    {
        QVector<T> fields;

        for (int i = 0; i < widget->count(); ++i) {
            const QListWidgetItem* item = widget->item(i);
            if (item->checkState() == Qt::Unchecked) {
                continue;
            }

            bool ok = false;
            const int value = item->data(Qt::UserRole).toInt(&ok);
            if (ok) {
                fields.push_back(static_cast<T>(value));
            }
        }
        return fields;
    }

    bool saveCsvToFile(const QString& type, QDir exportDir, const QString& csv);

    QString defaultCsvFileName(const QString& type) const;

private:
    Ui::CsvExportDialog* ui;
};
