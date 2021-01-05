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

    template<typename T>
    void openFileWithStream(QFile& file, T callback)
    {
        bool isOpen = openFileOrPrintError(file);
        if (!isOpen) {
            return;
        }
        QTextStream out(&file);
        // UTF-8 BOM required for e.g. Excel
        out.setCodec("UTF-8");
        out.setGenerateByteOrderMark(true);
        callback(out);
        // flush before closing the file or the data won't be written
        out.flush();
        file.close();
        m_shouldAbort = !checkTextStreamStatus(out);
    }


    bool openFileOrPrintError(QFile& file);
    bool checkTextStreamStatus(QTextStream& stream);
    QString exportFilePath(const QDir& dir, const QString& filename) const;
    QString defaultCsvFileName(const QString& type) const;

private:
    Ui::CsvExportDialog* ui;
    bool m_shouldAbort = false;
};
