#pragma once

#include "globals/Meta.h"

#include <QDebug>
#include <QMetaType>
#include <QString>
#include <QVector>

struct Actor
{
    QString name;
    QString role;
    QString thumb;
    QByteArray image;
    QString id;
    int order = 0; // used by Kodi NFO
    bool imageHasChanged = false;
};


class Actors : public QObject
{
    Q_OBJECT
public:
    Actors(QObject* parent = nullptr) : QObject(parent) {}
    ~Actors() override;

    void setActors(QVector<Actor> actors);
    void addActor(Actor actor);
    /// \brief Removes the given actor. Actors are compared by pointer and not by value.
    void removeActor(Actor* actor);

    elch_ssize_t size() { return m_actors.size(); }
    bool hasActors() const;

    /// \brief Clears all images from all actors.
    void clearImages();
    /// \brief Deletes all actors. All actor pointers are invalidated.
    void removeAll();

    const QVector<Actor*>& actors();
    QVector<const Actor*> actors() const;

public:
    auto begin() { return m_actors.begin(); }
    auto end() { return m_actors.end(); }
    auto begin() const { return m_actors.begin(); }
    auto end() const { return m_actors.end(); }
    auto cbegin() const { return m_actors.cbegin(); }
    auto cend() const { return m_actors.cend(); }

    auto back() { return m_actors.back(); }

private:
    QVector<Actor*> m_actors;
};


Q_DECLARE_METATYPE(Actor*)
Q_DECLARE_METATYPE(QString*)
Q_DECLARE_METATYPE(QVector<int>)

QDebug operator<<(QDebug dbg, const Actor& actor);
