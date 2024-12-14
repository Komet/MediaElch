#pragma once

#include "renamer/RenamerPlaceholders.h"
#include "ui/renamer/RenamerDialog.h"

#include <QWidget>

namespace Ui {
class RenamerPlaceholdersWidget;
}

class RenamerPlaceholdersWidget : public QWidget
{
    Q_OBJECT

public:
    explicit RenamerPlaceholdersWidget(QWidget* parent = nullptr);
    ~RenamerPlaceholdersWidget() override;
    void setPlaceholders(mediaelch::RenamerPlaceholders& renamerPlaceholders);

private:
    Ui::RenamerPlaceholdersWidget* ui;
};
