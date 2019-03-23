#pragma once

#include "globals/Globals.h"

#include <QDir>
#include <QString>
#include <QStringList>
#include <QVector>

/**
 * @brief The ConcertFileSearcher class
 */
class ConcertFileSearcher : public QObject
{
    Q_OBJECT
public:
    explicit ConcertFileSearcher(QObject* parent = nullptr);
    void setConcertDirectories(QVector<SettingsDir> directories);

public slots:
    void reload(bool force);
    void abort();

signals:
    void searchStarted(QString, int);
    void progress(int, int, int);
    void concertsLoaded(int);
    void currentDir(QString);

private:
    QVector<SettingsDir> m_directories;
    int m_progressMessageId;
    void scanDir(QString startPath,
        QString path,
        QVector<QStringList>& contents,
        bool separateFolders = false,
        bool firstScan = false);
    QStringList getFiles(QString path);
    bool m_aborted;
};
