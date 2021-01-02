#pragma once

#include <QDebug>
#include <QString>
#include <ostream>

class TheAudioDbId
{
public:
    TheAudioDbId() = default;
    explicit TheAudioDbId(QString TheAudioDbId);

    bool operator==(const TheAudioDbId& other) const;
    bool operator!=(const TheAudioDbId& other) const;

    QString toString() const;
    bool isValid() const;
    static bool isValidFormat(const QString& TheAudioDbId);

    static const TheAudioDbId NoId;

private:
    // can be replaced by std::optional<T> in C++17
    QString m_TheAudioDbId;
    bool m_isValid = false;
};

std::ostream& operator<<(std::ostream& os, const TheAudioDbId& id);
QDebug operator<<(QDebug debug, const TheAudioDbId& id);
