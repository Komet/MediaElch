#include "MovieFilesToDirs.h"
#include "MovieFileSearcher.h"

//#include <QApplication>
//#include <QDebug>
//#include "globals/Manager.h"

/**
 * @brief MovieFilesToDirs::MovieFilesToDirs
 * @param parent
 */
MovieFilesToDirs::MovieFilesToDirs(QObject *parent) :
    QThread(parent)
{
    m_progressMessageId = Constants::MovieFilesToDirsProgressMessageId;
}

/**
 * @brief MovieFilesToDirs::~MovieFilesToDirs
 */
MovieFilesToDirs::~MovieFilesToDirs()
{
}

/**
 * @brief Starts the foldering process
 */
void MovieFilesToDirs::run()
{
    emit progressStarted(tr("Putting movies to seperate folders..."), m_progressMessageId);

}
