#pragma once

#include <QDebug>
#include <QString>
#include <ostream>

class TvMazeId
{
public:
    TvMazeId() = default;
    explicit TvMazeId(QString tvmazeId);
    explicit TvMazeId(int tvmazeId);

    bool operator==(const TvMazeId& other) const;
    bool operator!=(const TvMazeId& other) const;

    QString toString() const;

    bool isValid() const;
    /// \brief Returns true if the given id has the common TVmaze ID format.
    /// \details A TVmaze ID is valid if it is numerical.
    static bool isValidFormat(const QString& tvmazeId);

    static const TvMazeId NoId;

private:
    QString m_tvmazeId;
};

std::ostream& operator<<(std::ostream& os, const TvMazeId& id);
QDebug operator<<(QDebug debug, const TvMazeId& id);
