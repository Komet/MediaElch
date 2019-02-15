#pragma once

#include "settings/ImportSettings.h"

#include <QMap>
#include <QObject>
#include <QProcess>
#include <QVector>

class MakeMkvCon : public QObject
{
    Q_OBJECT
public:
    explicit MakeMkvCon(ImportSettings& settings, QObject* parent = nullptr);
    ~MakeMkvCon() override;

    struct Track
    {
        QString name;
        int chapters;
        QString duration;
        int64_t size;
        QString fileName;
    };

public slots:
    void onGetDrives();
    void onScanDrive(int id);
    void onImportTrack(int trackId, int driveId, QString importFolder);
    void onBackupDisc(int driveId, QString importFolder);

signals:
    void sigGotDrives(QMap<int, QString>);
    void sigScannedDrive(QString, QMap<int, MakeMkvCon::Track>);
    void sigMessage(QString);
    void sigProgress(int, int);
    void sigTrackImported(int);
    void sigDiscBackedUp();

private slots:
    void onReadyRead();
    void onReadyReadError();
    void onFinished(int exitCode, QProcess::ExitStatus status);

private:
    ImportSettings& m_settings;
    QVector<QProcess*> m_processes;
    QMap<int, QString> m_drives;
    QMap<int, Track> m_tracks;
    QString m_lastOutput;
    QString m_title;
    void parseScanDrive(QString line);
    void parseInfo(QString line);
    void parseMsg(QString line);
    void parseProgress(QString line);
};
