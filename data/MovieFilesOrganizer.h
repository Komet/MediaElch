#ifndef MOVIEFILESORGANIZER_H
#define MOVIEFILESORGANIZER_H

#include <QThread>
#include <QDir>

#include "data/Movie.h"
#include "globals/Globals.h"

/**
 * @brief The MovieFilesOrganizer class
 */
class MovieFilesOrganizer : public QThread
{
    Q_OBJECT
public:
    explicit MovieFilesOrganizer(QObject *parent = 0);
    ~MovieFilesOrganizer();
    void canceled(QString msg);
    void moveToDirs(QString dir);

signals:
    void progressStarted(QString, int);
    void progress(int, int, int);
    void moviesOrganized(int);

private:
    //QList<SettingsDir> m_directories;
    int m_progressMessageId;
    //void getDirContents(QString path, QList<QStringList> &contents);
};

#endif // MOVIEFILESORGANIZER_H

