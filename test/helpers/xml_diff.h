#pragma once

#include <QDomDocument>
#include <QString>

#include "third_party/catch2/catch.hpp"

namespace test {

/// Parses a given XML string and fails using Catch2's REQUIRE macro if
/// the string can't be parsed.
QDomDocument parseXmlOrFail(const QString& content);

/// Checks whether both XML strings are the same. Fails otherwise.
/// If filename is given and the environment variable MEDIAELCH_UPDATE_REF_FILES is set
/// the original file will be overwritten if there are differences.
void compareXmlOrUpdateRef(const QString& expected, const QString& actual, const QString& filename);

} // namespace test
