#pragma once

#include "globals/Actor.h"
#include "globals/Globals.h"

#include <QString>
#include <QVector>
#include <memory>
#include <vector>

class MovieCrew
{
public:
    QString writer() const;
    QString director() const;
    QVector<Actor*> actors();
    QVector<const Actor*> actors() const;

    void setWriter(QString writer);
    void setDirector(QString director);
    void setActors(QVector<Actor> actors);
    void addActor(Actor actor);
    void removeActor(Actor* actor);

private:
    QString m_writer;
    QString m_director;
    /// Actors of this crew. Need to use a unique_ptr because some UI logic
    /// stores the address of the actor in some widget as Qt::UserRole...
    /// And QVector needs a default constructible type...
    std::vector<std::unique_ptr<Actor>> m_actors;
};
