#pragma once

#include "data/tv_show/TvShow.h"
#include "data/tv_show/TvShowEpisode.h"
#include "ui/renamer/RenamerDialog.h"

#include <QDialog>

class TvShowRenamerDialog : public RenamerDialog
{
    Q_OBJECT

public:
    explicit TvShowRenamerDialog(QWidget* parent = nullptr);
    ~TvShowRenamerDialog() override;
};
