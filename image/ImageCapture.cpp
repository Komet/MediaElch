#include "ImageCapture.h"

#include <QCoreApplication>
#include <QDebug>
#include <QProcess>
#include <QTemporaryFile>
#include "globals/Helper.h"
#include "notifications/NotificationBox.h"

ImageCapture::ImageCapture(QObject *parent) : QObject(parent)
{

}

bool ImageCapture::captureImage(QString file, StreamDetails *streamDetails, QImage &img)
{
    if (streamDetails->videoDetails().value("durationinseconds", 0) == 0)
        streamDetails->loadStreamDetails();
    if (streamDetails->videoDetails().value("durationinseconds", 0) == 0) {
        NotificationBox::instance()->showMessage(tr("Could not get duration of file"), NotificationBox::NotificationError);
        return false;
    }

    QString ffmpegbin = "ffmpeg";
#ifdef Q_OS_OSX
    ffmpegbin = QCoreApplication::applicationDirPath() + "/ffmpeg";
#elif defined(Q_OS_WIN)
    ffmpegbin = QCoreApplication::applicationDirPath() + "/vendor/ffmpeg.exe";
#endif

    QProcess ffmpeg;
    qsrand(QTime::currentTime().msec());
    int t = qrand()%streamDetails->videoDetails().value("durationinseconds", 0).toInt();
    QString timeCode = Helper::instance()->secondsToTimeCode(t);

    QTemporaryFile tmpFile;
    if (!tmpFile.open()) {
        NotificationBox::instance()->showMessage(tr("Temporary output file could not be opened"), NotificationBox::NotificationError);
        return false;
    }
    tmpFile.close();

    ffmpeg.start(ffmpegbin, QStringList() << "-y" << "-ss" << timeCode << "-i" << file << "-vframes" << "1" << "-q:v" << "2" << "-f" << "mjpeg" << tmpFile.fileName());
    if (!ffmpeg.waitForStarted()) {
#if defined(Q_OS_WIN) || defined(Q_OS_OSX)
        NotificationBox::instance()->showMessage(tr("Could not start ffmpeg"), NotificationBox::NotificationError);
#else
        NotificationBox::instance()->showMessage(tr("Could not start ffmpeg. Please install it and make it available in your $PATH"), NotificationBox::NotificationError);
#endif
        return false;
    }

    if (!ffmpeg.waitForFinished(10000)) {
        NotificationBox::instance()->showMessage(tr("ffmpeg did not finish"), NotificationBox::NotificationError);
        return false;
    }
    if (!tmpFile.open()) {
        NotificationBox::instance()->showMessage(tr("Temporary output file could not be opened"), NotificationBox::NotificationError);
        return false;
    }

    img = QImage::fromData(tmpFile.readAll());
    tmpFile.close();

    img = img.scaled(400, 300, Qt::KeepAspectRatio, Qt::SmoothTransformation);

    return true;
}
