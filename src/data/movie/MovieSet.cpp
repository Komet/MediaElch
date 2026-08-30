#include "data/movie/MovieSet.h"

#include <utility>

MovieSet MovieSet::renamedTo(QString newName) const
{
    MovieSet renamed = *this;
    renamed.name = std::move(newName);
    return renamed;
}
