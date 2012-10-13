#ifndef CONCERTFILESEARCHER_H
#define CONCERTFILESEARCHER_H

#include <QDir>
#include <QObject>
#include <QThread>

#include "data/Concert.h"
#include "globals/Globals.h"

/**
 * @brief The ConcertFileSearcher class
 */
class ConcertFileSearcher : public QThread
{
    Q_OBJECT
public:
    explicit ConcertFileSearcher(QObject *parent = 0);

    void run();
    void setConcertDirectories(QList<SettingsDir> directories);

signals:
    void searchStarted(QString, int);
    void progress(int, int, int);
    void concertsLoaded(int);

private:
    QList<SettingsDir> m_directories;
    int m_progressMessageId;
    void scanDir(QString path, QList<QStringList> &contents, bool separateFolders = false, bool firstScan = false);
    QStringList getCachedFiles(QString path);
};

#endif // CONCERTFILESEARCHER_H
