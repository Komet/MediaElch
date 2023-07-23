#pragma once

#include <QString>

namespace mediaelch {

void removeUnicodeSpaces(QString& str);

QString removeHtmlEntities(const QString& str);

/// \brief Removes HTML entities from the string and normalizes it.
/// \details
///   On top of remove HTML entities such as `<p>`, it normalizes
///   e.g. non-breaking-spaces to normal spaces.  Use this function
///   if you want to use content from an HTML page.
QString normalizeFromHtml(const QString& str);

} // namespace mediaelch
