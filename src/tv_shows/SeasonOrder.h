#pragma once

#include <ostream>

enum class SeasonOrder : int
{
    Aired = 1,
    Dvd = 2
};

std::ostream& operator<<(std::ostream& os, const SeasonOrder& order);
