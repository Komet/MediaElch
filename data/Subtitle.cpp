#include "Subtitle.h"

Subtitle::Subtitle(QObject *parent) : QObject(parent)
{
    m_changed = false;
}

QString Subtitle::language() const
{
    return m_language;
}

void Subtitle::setLanguage(const QString &language)
{
    m_language = language;
    setChanged(true);
}

QStringList Subtitle::files() const
{
    return m_files;
}

void Subtitle::setFiles(const QStringList &files, bool emitChanged)
{
    m_files = files;
    if (emitChanged)
        setChanged(true);
}

bool Subtitle::forced() const
{
    return m_forced;
}

void Subtitle::setForced(bool forced)
{
    m_forced = forced;
    setChanged(true);
}

bool Subtitle::changed() const
{
    return m_changed;
}

void Subtitle::setChanged(bool changed)
{
    m_changed = changed;
    if (changed)
        emit sigChanged();
}

