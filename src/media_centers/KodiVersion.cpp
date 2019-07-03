#include "media_centers/KodiVersion.h"

namespace mediaelch {

KodiVersion::KodiVersion(int version) : m_version(fromInt(version))
{
}

KodiVersion KodiVersion::latest()
{
    return KodiVersion(); // default constructor uses latest version
}

bool KodiVersion::isValid(int val)
{
    return val >= 16 && val <= 18;
}

QVector<KodiVersion> KodiVersion::all()
{
    return {KodiVersion(v16), KodiVersion(v17), KodiVersion(v18)};
}

KodiVersion::Version KodiVersion::fromInt(int version)
{
    if (version == 16) {
        return v16;
    }
    if (version == 17) {
        return v17;
    }
    return v18;
}

int KodiVersion::toInt() const
{
    return static_cast<int>(m_version);
}

QString KodiVersion::toString() const
{
    return QString::number(toInt());
}

KodiVersion::Version KodiVersion::version() const
{
    return m_version;
}

} // namespace mediaelch
