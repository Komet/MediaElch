#include "media_center/KodiVersion.h"

namespace mediaelch {

KodiVersion::KodiVersion(int version) : m_version(fromInt(version))
{
}

KodiVersion KodiVersion::latest()
{
    return {}; // default constructor uses latest version
}

bool KodiVersion::isValid(int val)
{
    return val >= 17 && val <= 22;
}

QVector<KodiVersion> KodiVersion::all()
{
    return {KodiVersion(v17), KodiVersion(v18), KodiVersion(v19), KodiVersion(v20), KodiVersion(v21), KodiVersion(v22)};
}

KodiVersion::Version KodiVersion::fromInt(int version)
{
    if (version == 17) {
        return v17;
    } else if (version == 18) {
        return v18;
    } else if (version == 19) {
        return v19;
    } else if (version == 20) {
        return v20;
    } else if (version == 21) {
        return v21;
    } else if (version == 22) {
        return v22;
    } else {
        return v20; // default
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
