#pragma once

#include "renamer/Renamer.h"

class RenamerDialog;
class Movie;

class MovieRenamer : public Renamer
{
public:
    MovieRenamer(RenamerConfig renamerConfig, RenamerDialog* dialog);
    RenameError renameMovie(Movie& movie);
};
