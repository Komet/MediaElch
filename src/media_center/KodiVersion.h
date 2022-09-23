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
        v17 = 17,
        v18 = 18,
        v19 = 19
    };

    explicit KodiVersion(Version version = v19) : m_version(version) {}
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
