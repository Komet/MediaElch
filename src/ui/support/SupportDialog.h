#pragma once

#include <QDialog>

namespace Ui {
class SupportDialog;
}

/**
 * @brief The SupportDialog class
 */
class SupportDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SupportDialog(QWidget* parent = nullptr);
    ~SupportDialog() override;

private slots:
    void onAlreadyDonated();

private:
    Ui::SupportDialog* ui;
};
