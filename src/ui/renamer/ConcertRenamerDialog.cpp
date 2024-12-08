#include "ui/renamer/ConcertRenamerDialog.h"

ConcertRenamerDialog::ConcertRenamerDialog(QWidget* parent) : RenamerDialog(parent)
{
    m_renameType = RenameType::Concerts;
}

ConcertRenamerDialog::~ConcertRenamerDialog() = default;
