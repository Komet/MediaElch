#pragma once

#include <QDebug>
#include <QString>
#include <ostream>

class ImdbId
{
public:
    ImdbId() = default;
    explicit ImdbId(QString imdbId);

    bool operator==(const ImdbId& other) const;
    bool operator!=(const ImdbId& other) const;

    QString toString() const;
    bool isValid() const;
    static bool isValidFormat(const QString& imdbId);

    static const ImdbId NoId;

private:
    // can be replaced by std::optional<T> in C++17
    QString m_imdbId;
    bool m_isValid{false};
};

std::ostream& operator<<(std::ostream& os, const ImdbId& id);
QDebug operator<<(QDebug debug, const ImdbId& id);
