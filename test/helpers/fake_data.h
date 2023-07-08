#pragma once

#include "data/movie/Movie.h"

#include <memory>

namespace test {

std::unique_ptr<Movie> movieWithAllDetails();

} // namespace test
