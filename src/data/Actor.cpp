#include "data/Actor.h"


QDebug operator<<(QDebug dbg, const Actor& actor)
{
    QString nl = "\n";
    QString out;
    out.append("Actor").append(nl);
    out.append(QStringLiteral("  Name:  ").append(actor.name).append(nl));
    out.append(QStringLiteral("  Role:  ").append(actor.role).append(nl));
    out.append(QStringLiteral("  Thumb: ").append(actor.thumb).append(nl));
    out.append(QStringLiteral("  ID:    ").append(actor.id).append(nl));
    out.append(QStringLiteral("  Order: ").append(QString::number(actor.order)).append(nl));
    dbg.nospace() << out;
    return dbg.maybeSpace();
}
