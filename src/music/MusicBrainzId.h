#pragma once

#include <QDebug>
#include <QString>
#include <ostream>

class MusicBrainzId
{
public:
    MusicBrainzId() = default;
    explicit MusicBrainzId(QString musicBrainzId);

    bool operator==(const MusicBrainzId& other) const;
    bool operator!=(const MusicBrainzId& other) const;

    QString toString() const;
    bool isValid() const;
    static bool isValidFormat(const QString& musicBrainzId);

    static const MusicBrainzId NoId;

private:
    // can be replaced by std::optional<T> in C++17
    QString m_musicBrainzId;
    bool m_isValid = false;
};

std::ostream& operator<<(std::ostream& os, const MusicBrainzId& id);
QDebug operator<<(QDebug debug, const MusicBrainzId& id);
