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

    static const TvDbId NoId;

private:
    QString m_tvdbId;
};

std::ostream& operator<<(std::ostream& os, const TvDbId& id);
