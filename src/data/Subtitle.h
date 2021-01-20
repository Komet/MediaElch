#pragma once

#include <QObject>
#include <QStringList>

class Subtitle : public QObject
{
    Q_OBJECT
public:
    explicit Subtitle(QObject* parent = nullptr);

    const QString& language() const;
    void setLanguage(const QString& language);

    const QStringList& files() const;
    void setFiles(const QStringList& files, bool emitChanged = true);

    /// Whether subtitles are focred, i.e. "burned" into the image.
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
