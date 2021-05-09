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

void Actors::addActor(Actor actor)
{
    if (actor.order == 0 && !m_actors.empty()) {
        actor.order = m_actors.back()->order + 1;
    }
    auto* a = new Actor(actor);
    m_actors.push_back(a);
}

void Actors::removeActor(Actor* actor)
{
    auto i = std::find(m_actors.begin(), m_actors.end(), actor);
    if (i != m_actors.end()) {
        m_actors.erase(i);
        delete actor;
    }
}

bool Actors::hasActors() const
{
    return !m_actors.isEmpty();
}

void Actors::clearImages()
{
    for (auto& actor : m_actors) {
        actor->image = QByteArray();
    }
}

Actors::~Actors()
{
    qDeleteAll(m_actors);
    m_actors.clear();
}

void Actors::setActors(QVector<Actor> actors)
{
    removeAll();
    for (const Actor& a : actors) {
        auto* actor = new Actor(a);
        m_actors.push_back(actor);
    }
}

void Actors::removeAll()
{
    qDeleteAll(m_actors);
    m_actors.clear();
}

const QVector<Actor*>& Actors::actors()
{
    return m_actors;
}

QVector<const Actor*> Actors::actors() const
{
    QVector<Actor const*> actors;
    for (Actor const* a : m_actors) {
        actors << a;
    }
    return actors;
}
