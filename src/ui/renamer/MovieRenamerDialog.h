#pragma once

#include "data/movie/Movie.h"
#include "ui/renamer/RenamerDialog.h"

#include <QDialog>

class MovieRenamerDialog : public RenamerDialog
{
    Q_OBJECT

public:
    explicit MovieRenamerDialog(QWidget* parent = nullptr);
    ~MovieRenamerDialog() override;
};
