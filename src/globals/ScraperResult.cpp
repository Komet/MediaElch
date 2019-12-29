#include "globals/ScraperResult.h"

QDebug operator<<(QDebug lhs, const ScraperSearchResult& rhs)
{
    lhs << QString(R"(("%1", "%2", %3))").arg(rhs.id, rhs.name, rhs.released.toString("yyyy"));
    return lhs;
}
