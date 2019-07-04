#pragma once

#include <QDomDocument>
#include <QString>

#include "third_party/catch2/catch.hpp"

/// Parses a given XML string and fails using Catch2's REQUIRE macro if
/// the string can't be parsed.
QDomDocument parseXml(const QString& content);

/// Prints differences in two QDomDocuments.
void diffDom(const QDomDocument& expected, const QDomDocument& actual);

/// Checks whether both XML strings are the same. Fails otherwise.
void checkSameXml(const QString& expected, const QString& actual);
