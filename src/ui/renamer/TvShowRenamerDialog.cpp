#include "ui/renamer/TvShowRenamerDialog.h"

TvShowRenamerDialog::TvShowRenamerDialog(QWidget* parent) : RenamerDialog(parent)
{
    m_renameType = RenameType::TvShows;
}

TvShowRenamerDialog::~TvShowRenamerDialog() = default;
