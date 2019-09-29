#pragma once

#include <QDebug>
#include <QString>
#include <QStringList>

namespace mediaelch {

/// Represents a MediaElch version. Useful for comparing versions.
class VersionInfo
{
public:
    VersionInfo() = default;

    /// Constructs a new version object and validates it.
    /// @param version MediaElch version, pattern: major[.minor[.patch]]
    explicit VersionInfo(QString version);

    /// Returns true if a version is valid. Versions starting from 0.0.1 are valid.
    /// 0.0.0 is a special "invalid" version
    bool isValid() const { return m_isValid; }

    /// Returns true if a version is stable, i.e. not a development version.
    /// The stable/unstable versioning pattern was introduced in v2.4.3.
    /// Prior versions are always regarded as valid. Invalid versions are unstable.
    ///
    /// A version is stable if the patch version is even, i.e. `patch % 2 == 0`
    bool isStable() const;
    bool isUnstable() const { return !isStable(); }

    /// Returns a string representation of the version in the format major.minor.patch
    /// If the version is invalid, an empty string is returned.
    QString toString() const;

    bool operator!=(const VersionInfo& rhs) const;
    // comparison of invalid version always fail (similar to NaN)
    bool operator==(const VersionInfo& rhs) const;
    bool operator<(const VersionInfo& rhs) const;
    bool operator<=(const VersionInfo& rhs) const;
    bool operator>(const VersionInfo& rhs) const;
    bool operator>=(const VersionInfo& rhs) const;

private:
    void setVersion(int major, int minor, int patch);

    uint8_t m_versionMajor = 0;
    uint8_t m_versionMinor = 0;
    uint8_t m_versionPatch = 0;
    bool m_isValid = false;
};

inline std::ostream& operator<<(std::ostream& os, const VersionInfo& version)
{
    return os << (version.isValid() ? version.toString().toStdString() : "invalid version");
}

inline QDebug operator<<(QDebug debug, const VersionInfo& version)
{
    QDebugStateSaver saver(debug);
    debug.nospace() << "Version(" << version.toString() << ')';
    return debug;
}

} // namespace mediaelch
