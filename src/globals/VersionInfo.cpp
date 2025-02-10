#include "globals/VersionInfo.h"

namespace mediaelch {

VersionInfo::VersionInfo(QString version)
{
    QStringList versionParts = version.split('.');
    int major = 0;
    int minor = 0;
    int patch = 0;

    if (!versionParts.isEmpty()) {
        major = versionParts[0].toInt(&m_isValid);
    }
    if (m_isValid && versionParts.size() >= 2) {
        minor = versionParts[1].toInt(&m_isValid);
    }
    if (m_isValid && versionParts.size() >= 3) {
        patch = versionParts[2].toInt(&m_isValid);
    }
    if (m_isValid) {
        setVersion(major, minor, patch);
    }
}

bool VersionInfo::isStable() const
{
    return isValid() && ((*this <= VersionInfo("2.4.2")) || (m_versionPatch % 2 == 0));
}

QString VersionInfo::toString() const
{
    if (!isValid()) {
        return "";
    }
    return QString::number(m_versionMajor) + '.' + QString::number(m_versionMinor) + '.'
           + QString::number(m_versionPatch);
}

bool VersionInfo::operator==(const VersionInfo& rhs) const
{
    return (isValid() && rhs.isValid() && (m_versionMajor == rhs.m_versionMajor)
            && (m_versionMinor == rhs.m_versionMinor) && (m_versionPatch == rhs.m_versionPatch));
}

bool VersionInfo::operator!=(const VersionInfo& rhs) const
{
    return !isValid() || !rhs.isValid() || !(*this == rhs);
}

bool VersionInfo::operator>(const VersionInfo& rhs) const
{
    // don't use ternary operator for readability

    if (!isValid() || !rhs.isValid()) {
        return false;
    }

    if (m_versionMajor > rhs.m_versionMajor) {
        return true;
    }
    if (m_versionMajor < rhs.m_versionMajor) {
        return false;
    }

    // major version same
    if (m_versionMinor > rhs.m_versionMinor) {
        return true;
    }
    if (m_versionMinor < rhs.m_versionMinor) {
        return false;
    }

    // minor version same
    if (m_versionPatch > rhs.m_versionPatch) {
        return true;
    }
    if (m_versionPatch < rhs.m_versionPatch) {
        return false;
    }

    // versions are the same
    return false;
}

bool VersionInfo::operator>=(const VersionInfo& rhs) const
{
    return isValid() && rhs.isValid() && (*this == rhs || *this > rhs);
}

bool VersionInfo::operator<(const VersionInfo& rhs) const
{
    return isValid() && rhs.isValid() && !(*this > rhs || *this == rhs);
}

bool VersionInfo::operator<=(const VersionInfo& rhs) const
{
    return isValid() && rhs.isValid() && (*this == rhs || *this < rhs);
}

void VersionInfo::setVersion(int major, int minor, int patch)
{
    // bounds check
    // 0.0.0 is a special version (invalid)
    m_isValid = (major >= 0 && minor >= 0 && patch >= 0) && //
                (major != 0 || minor != 0 || patch != 0) && //
                (major < 256 && minor < 256 && patch < 256);

    if (m_isValid) {
        m_versionMajor = static_cast<uint8_t>(major);
        m_versionMinor = static_cast<uint8_t>(minor);
        m_versionPatch = static_cast<uint8_t>(patch);
    }
}

} // namespace mediaelch
