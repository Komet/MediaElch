#include "MovieCrew.h"


QString MovieCrew::writer() const
{
    return m_writer;
}

QString MovieCrew::director() const
{
    return m_director;
}

QVector<Actor*> MovieCrew::actors()
{
    QVector<Actor*> actorPtrs;
    for (const auto& actor : m_actors) {
        actorPtrs.push_back(actor.get());
    }
    return actorPtrs;
}

QVector<const Actor*> MovieCrew::actors() const
{
    QVector<const Actor*> actorPtrs;
    for (const auto& actor : m_actors) {
        actorPtrs.push_back(actor.get());
    }
    return actorPtrs;
}

void MovieCrew::setWriter(QString writer)
{
    m_writer = writer;
}

void MovieCrew::setDirector(QString director)
{
    m_director = director;
}

void MovieCrew::setActors(QVector<Actor> actors)
{
    m_actors.clear();
    for (const Actor& actor : actors) {
        m_actors.push_back(std::make_unique<Actor>(actor));
    }
}

void MovieCrew::addActor(Actor actor)
{
    if (actor.order == 0 && !m_actors.empty()) {
        actor.order = m_actors.back()->order + 1;
    }
    m_actors.push_back(std::make_unique<Actor>(actor));
}

void MovieCrew::removeActor(Actor* actor)
{
    for (int i = 0, n = m_actors.size(); i < n; ++i) {
        if (m_actors[i].get() == actor) {
            m_actors.erase(m_actors.begin() + i);
            break;
        }
    }
}
