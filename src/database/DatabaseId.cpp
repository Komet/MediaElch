#include "database/DatabaseId.h"


QDebug operator<<(QDebug dbg, const mediaelch::DatabaseId& db)
{
    dbg.nospace() << "DbId(" << db.id << ")";
    return dbg.maybeSpace();
}
