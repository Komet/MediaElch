#include "media_centers/kodi/EpisodeXmlWriter.h"

#include "media_centers/kodi/EpisodeXmlReader.h"

#include <QString>

namespace mediaelch {
namespace kodi {

QString EpisodeXmlWriter::getEpisodeXmlWithSingleRoot(bool testMode)
{
    return EpisodeXmlReader::makeValidEpisodeXml(getEpisodeXml(testMode));
}

} // namespace kodi
} // namespace mediaelch
