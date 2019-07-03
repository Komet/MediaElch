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
        v16 = 16,
        v17 = 17,
        v18 = 18
    };

    explicit KodiVersion(Version version = v18) : m_version(version) {}
    explicit KodiVersion(int version);

    static KodiVersion latest();
    static bool isValid(int val);
    static QVector<KodiVersion> all();

    int toInt() const;
    QString toString() const;
    Version version() const;

private:
    Version fromInt(int version);
    Version m_version = v18;
};

} // namespace mediaelch
