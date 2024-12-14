#pragma once

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
    void setType(RenameType renameType);

private:
    Ui::RenamerPlaceholdersWidget* ui;
};
