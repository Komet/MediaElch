#pragma once

#include <QDebug>
#include <QString>
#include <ostream>

/// \brief ID for Wikidata, see <https://www.wikidata.org/wiki/Wikidata:Identifiers>
class WikidataId
{
public:
    WikidataId() = default;
    explicit WikidataId(QString wikidataId);

    bool operator==(const WikidataId& other) const;
    bool operator!=(const WikidataId& other) const;

    QString toString() const;
    bool isValid() const;
    /// \brief Returns true if the given id has the common Wikidata ID format.
    /// \details A Wikidata ID is valid if it starts with "Q".
    static bool isValidPrefixedFormat(const QString& wikidataId);

    static const WikidataId NoId;

private:
    QString m_wikidataId;
};

std::ostream& operator<<(std::ostream& os, const WikidataId& id);
QDebug operator<<(QDebug debug, const WikidataId& id);
