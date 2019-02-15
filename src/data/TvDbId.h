#pragma once

#include <QString>

class TvDbId
{
public:
    TvDbId() = default;
    explicit TvDbId(QString tvdbId);
    explicit TvDbId(int tvdbId);

    bool operator==(const TvDbId& other);
    bool operator!=(const TvDbId& other);

    QString toString() const;
    QString withPrefix() const;
    bool isValid() const;

    static const TvDbId NoId;

private:
    QString m_tvdbId;
};
