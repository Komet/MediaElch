#pragma once

#define CATCH_CONFIG_ENABLE_CHRONO_STRINGMAKER

// Catch2 is included in each test cpp file
#include "third_party/catch2/catch.hpp"

#include "test/helpers/debug_output.h"
#include "test/helpers/matchers.h"
#include "test/helpers/xml_diff.h"

#include "globals/Globals.h"
#include "globals/ScraperResult.h"
#include "movies/Movie.h"
#include "movies/MovieController.h"

#include <QEventLoop>
#include <QMap>
#include <QString>
#include <QStringList>
#include <QVector>
