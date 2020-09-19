#include "globals/Actor.h"


QDebug operator<<(QDebug dbg, const Actor& actor)
{
    QString nl = "\n";
    QString out;
    out.append("Actor").append(nl);
    out.append(QString("  Name:  ").append(actor.name).append(nl));
    out.append(QString("  Role:  ").append(actor.role).append(nl));
    out.append(QString("  Thumb: ").append(actor.thumb).append(nl));
    out.append(QString("  ID:    ").append(actor.id).append(nl));
    out.append(QString("  Order: ").append(actor.order).append(nl));
    dbg.nospace() << out;
    return dbg.maybeSpace();
}
