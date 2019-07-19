#pragma once

#include "renamer/Renamer.h"

class RenamerDialog;
class Concert;

class ConcertRenamer : public Renamer
{
public:
    ConcertRenamer(RenamerConfig renamerConfig, RenamerDialog* dialog);
    RenameError renameConcert(Concert& concert);
};
