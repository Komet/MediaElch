#pragma once

#include "ui/renamer/RenamerDialog.h"

#include <QWidget>

namespace Ui {
class RenamerPlaceholders;
}

class RenamerPlaceholders : public QWidget
{
    Q_OBJECT

public:
    explicit RenamerPlaceholders(QWidget* parent = nullptr);
    ~RenamerPlaceholders() override;
    void setType(RenameType renameType);

private:
    Ui::RenamerPlaceholders* ui;
};
