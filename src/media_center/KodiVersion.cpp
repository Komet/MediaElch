#include "media_center/KodiVersion.h"

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
    return val >= 17 && val <= 20;
}

QVector<KodiVersion> KodiVersion::all()
{
    return {KodiVersion(v17), KodiVersion(v18), KodiVersion(v19), KodiVersion(v20)};
}

KodiVersion::Version KodiVersion::fromInt(int version)
{
    if (version == 17) {
        return v17;
    } else if (version == 18) {
        return v18;
    } else if (version == 19) {
        return v19;
    } else {
        return v20;
    }
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
