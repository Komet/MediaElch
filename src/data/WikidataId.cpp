#include "WikidataId.h"

#include <QRegularExpression>
#include <QString>
#include <utility>

WikidataId::WikidataId(QString wikidataId) : m_wikidataId(std::move(wikidataId))
{
}

const WikidataId WikidataId::NoId = WikidataId();

bool WikidataId::operator==(const WikidataId& other) const
{
    return m_wikidataId == other.m_wikidataId;
}

bool WikidataId::operator!=(const WikidataId& other) const
{
    return !(*this == other);
}

QString WikidataId::toString() const
{
    return m_wikidataId;
}

bool WikidataId::isValid() const
{
    static QRegularExpression rx("^Q\\d+$");
    return rx.match(m_wikidataId).hasMatch();
}

std::ostream& operator<<(std::ostream& os, const WikidataId& id)
{
    return os << id.toString().toStdString();
}

QDebug operator<<(QDebug debug, const WikidataId& id)
{
    QDebugStateSaver saver(debug);
    debug.nospace() << "WikidataId(" << id.toString() << ')';
    return debug;
}
