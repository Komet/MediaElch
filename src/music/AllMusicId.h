#pragma once

#include <QDebug>
#include <QString>
#include <ostream>

class AllMusicId
{
public:
    AllMusicId() = default;
    explicit AllMusicId(QString allMusicId);

    bool operator==(const AllMusicId& other) const;
    bool operator!=(const AllMusicId& other) const;

    QString toString() const;
    bool isValid() const;
    static bool isValidFormat(const QString& allMusicId);

    static const AllMusicId NoId;

private:
    // can be replaced by std::optional<T> in C++17
    QString m_allMusicId;
    bool m_isValid = false;
};

std::ostream& operator<<(std::ostream& os, const AllMusicId& id);
QDebug operator<<(QDebug debug, const AllMusicId& id);
