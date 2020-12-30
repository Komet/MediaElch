#pragma once

#include "media_centers/KodiVersion.h"

#include <QDomDocument>
#include <QString>
#include <QXmlStreamWriter>

namespace mediaelch {
namespace kodi {

/// \brief Adds a xml <generator> tag to the given document.
/// \details The <generator> tag contains MediaElch specific values and can be
///          used for debugging purposes.
void addMediaelchGeneratorTag(QDomDocument& doc, KodiVersion::Version kodiVersion);

/// \brief Writes a xml <generator> tag to the given document.
/// \details The <generator> tag contains MediaElch specific values and can be
///          used for debugging purposes.
void addMediaelchGeneratorTag(QXmlStreamWriter& xml, KodiVersion::Version kodiVersion);

} // namespace kodi
} // namespace mediaelch
