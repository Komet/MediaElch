#include "media_centers/kodi/EpisodeXmlWriter.h"

#include "media_centers/kodi/EpisodeXmlReader.h"

#include <QString>

namespace mediaelch {
namespace kodi {

QString EpisodeXmlWriter::getEpisodeXmlWithSingleRoot()
{
    return EpisodeXmlReader::makeValidEpisodeXml(getEpisodeXml());
}

} // namespace kodi
} // namespace mediaelch
