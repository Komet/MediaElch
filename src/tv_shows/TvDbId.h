#pragma once

#include <QDebug>
#include <QString>
#include <ostream>

class TvDbId
{
public:
    TvDbId() = default;
    explicit TvDbId(QString tvdbId);
    explicit TvDbId(int tvdbId);

    bool operator==(const TvDbId& other) const;
    bool operator!=(const TvDbId& other) const;

    QString toString() const;
    QString withPrefix() const;
    bool isValid() const;
    /// \brief Returns true if the given id has the common TheTvDb ID format.
    /// \details A TheTvDb ID is valid if it starts with "id".
    static bool isValidPrefixedFormat(const QString& tvdbId);

    static const TvDbId NoId;

private:
    QString m_tvdbId;
};

std::ostream& operator<<(std::ostream& os, const TvDbId& id);
QDebug operator<<(QDebug debug, const TvDbId& id);
