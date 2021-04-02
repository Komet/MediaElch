#pragma once

#include <QDialog>

namespace Ui {
class AboutDialog;
} // namespace Ui

class AboutDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AboutDialog(QWidget* parent = nullptr);
    ~AboutDialog() override;

private slots:
    void copyToClipboard();

private:
    void setDeveloperDetails();
    void setLibraryDetails();
    Ui::AboutDialog* ui;
};
