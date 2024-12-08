#pragma once

#include "data/concert/Concert.h"
#include "ui/renamer/RenamerDialog.h"

#include <QDialog>

class ConcertRenamerDialog : public RenamerDialog
{
    Q_OBJECT

public:
    explicit ConcertRenamerDialog(QWidget* parent = nullptr);
    ~ConcertRenamerDialog() override;
};
