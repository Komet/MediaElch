#pragma once

#include <QDialog>

namespace Ui {
class AboutDialog;
} // namespace Ui

/**
 * @brief The AboutDialog class
 */
class AboutDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AboutDialog(QWidget* parent = nullptr);
    ~AboutDialog() override;

public slots:
    int exec() override;

private:
    void setDeveloperInformation();
    Ui::AboutDialog* ui;
};
