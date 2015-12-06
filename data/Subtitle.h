#ifndef SUBTITLE_H
#define SUBTITLE_H

#include <QObject>
#include <QStringList>

class Subtitle : public QObject
{
    Q_OBJECT
public:
    explicit Subtitle(QObject *parent = 0);

    QString language() const;
    void setLanguage(const QString &language);

    QStringList files() const;
    void setFiles(const QStringList &files, bool emitChanged = true);

    bool forced() const;
    void setForced(bool forced);

    bool changed() const;
    void setChanged(bool changed);

signals:
    void sigChanged();

private:
    QString m_language;
    QStringList m_files;
    bool m_forced;
    bool m_changed;
};

#endif // SUBTITLE_H
