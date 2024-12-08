#include "ui/renamer/MovieRenamerDialog.h"

MovieRenamerDialog::MovieRenamerDialog(QWidget* parent) : RenamerDialog(parent)
{
    m_renameType = RenameType::Movies;
}

MovieRenamerDialog::~MovieRenamerDialog() = default;
