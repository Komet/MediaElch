#ifndef CONCERT_RENAMER_H
#define CONCERT_RENAMER_H

#include "renamer/Renamer.h"

class RenamerDialog;
class Concert;

class ConcertRenamer : public Renamer
{
public:
    ConcertRenamer(RenamerConfig renamerConfig, RenamerDialog *dialog);
    RenameError renameConcert(Concert &movie);
};

#endif // CONCERT_RENAMER_H
