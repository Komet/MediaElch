#ifndef CONCERTFILESEARCHER_H
#define CONCERTFILESEARCHER_H

#include <QDir>
#include <QObject>

#include "data/Concert.h"
#include "globals/Globals.h"

/**
 * @brief The ConcertFileSearcher class
 */
class ConcertFileSearcher : public QObject
{
    Q_OBJECT
public:
    explicit ConcertFileSearcher(QObject *parent = 0);
    void setConcertDirectories(QList<SettingsDir> directories);

public slots:
    void run();

signals:
    void searchStarted(QString, int);
    void progress(int, int, int);
    void concertsLoaded(int);
    void currentDir(QString);

private:
    QList<SettingsDir> m_directories;
    int m_progressMessageId;
    void scanDir(QString startPath, QString path, QList<QStringList> &contents, bool separateFolders = false, bool firstScan = false);
    QStringList getCachedFiles(QString path);
};

#endif // CONCERTFILESEARCHER_H
