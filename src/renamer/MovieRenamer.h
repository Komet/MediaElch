#ifndef MOVIE_RENAMER_H
#define MOVIE_RENAMER_H

#include "renamer/Renamer.h"

class RenamerDialog;
class Movie;

class MovieRenamer : public Renamer
{
public:
    MovieRenamer(RenamerConfig renamerConfig, RenamerDialog *dialog);
    RenameError renameMovie(Movie &movie);
};

#endif // MOVIE_RENAMER_H
