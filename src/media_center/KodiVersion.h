#pragma once

#include <QVector>

namespace mediaelch {

// Kodi version represents a version of the Kodi media center API
class KodiVersion
{
public:
    // not an enum class so that we can use KodiVersion::v18
    enum Version : int
    {
        v17 = 17, ///< Krypton
        v18 = 18, ///< Leia
        v19 = 19, ///< Matrix
        v20 = 20, ///< Nexus
        v21 = 21, ///< Omega
        v22 = 22, ///< ?
        // when adding new values, also adapt:
        // isValid(), all(), fromInt() and constructor
    };

    /* implicit */ KodiVersion(Version version = v20) : m_version(version) {}
    explicit KodiVersion(int version);

    static KodiVersion latest();
    static bool isValid(int val);
    static QVector<KodiVersion> all();

    int toInt() const;
    QString toString() const;
    Version version() const;

private:
    Version fromInt(int version);
    Version m_version = v19;
};

} // namespace mediaelch
