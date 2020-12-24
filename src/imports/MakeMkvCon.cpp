#include "MakeMkvCon.h"

#include "settings/Settings.h"

MakeMkvCon::MakeMkvCon(ImportSettings& settings, QObject* parent) : QObject(parent), m_settings{settings}
{
}

MakeMkvCon::~MakeMkvCon()
{
    for (QProcess* process : asConst(m_processes)) {
        process->kill();
    }
}

void MakeMkvCon::onGetDrives()
{
    m_drives.clear();

    QStringList parameters;
    parameters << "-r"
               << "--cache=1"
               << "info"
               << "disc:9999";

    auto* process = new QProcess(this);
    m_processes.append(process);
    connect(process, &QProcess::readyReadStandardOutput, this, &MakeMkvCon::onReadyRead);
    connect(process, &QProcess::readyReadStandardError, this, &MakeMkvCon::onReadyReadError);
    connect(process, elchOverload<int, QProcess::ExitStatus>(&QProcess::finished), this, &MakeMkvCon::onFinished);
    process->start(m_settings.makeMkvCon(), parameters);
    process->setProperty("job", "scanDrives");
    process->waitForStarted(10000);
}

void MakeMkvCon::onScanDrive(int id)
{
    m_title.clear();
    m_tracks.clear();

    QStringList parameters;
    parameters << "-r"
               << "info" << QString("disc:%1").arg(id);

    auto* process = new QProcess(this);
    m_processes.append(process);
    connect(process, &QProcess::readyReadStandardOutput, this, &MakeMkvCon::onReadyRead);
    connect(process, &QProcess::readyReadStandardError, this, &MakeMkvCon::onReadyReadError);
    connect(process, elchOverload<int, QProcess::ExitStatus>(&QProcess::finished), this, &MakeMkvCon::onFinished);
    process->start(m_settings.makeMkvCon(), parameters);
    process->setProperty("job", "info");
    process->waitForStarted(10000);
}

void MakeMkvCon::onImportTrack(int trackId, int driveId, QString importFolder)
{
    QStringList parameters;
    parameters << "-r"
               << "--progress=-stdout"
               << "mkv" << QString("disc:%1").arg(driveId) << QString("%1").arg(trackId) << importFolder;

    auto* process = new QProcess(this);
    m_processes.append(process);
    connect(process, &QProcess::readyReadStandardOutput, this, &MakeMkvCon::onReadyRead);
    connect(process, &QProcess::readyReadStandardError, this, &MakeMkvCon::onReadyReadError);
    connect(process, elchOverload<int, QProcess::ExitStatus>(&QProcess::finished), this, &MakeMkvCon::onFinished);
    process->start(m_settings.makeMkvCon(), parameters);
    process->setProperty("job", "importTrack");
    process->setProperty("trackId", trackId);
    process->waitForStarted(10000);
}

void MakeMkvCon::onBackupDisc(int driveId, QString importFolder)
{
    QStringList parameters;
    parameters << "-r"
               << "--progress=-stdout"
               << "backup" << QString("disc:%1").arg(driveId) << importFolder;

    auto* process = new QProcess(this);
    m_processes.append(process);
    connect(process, &QProcess::readyReadStandardOutput, this, &MakeMkvCon::onReadyRead);
    connect(process, &QProcess::readyReadStandardError, this, &MakeMkvCon::onReadyReadError);
    connect(process, elchOverload<int, QProcess::ExitStatus>(&QProcess::finished), this, &MakeMkvCon::onFinished);
    process->start(m_settings.makeMkvCon(), parameters);
    process->setProperty("job", "backupDisc");
    process->waitForStarted(10000);
}

void MakeMkvCon::onReadyRead()
{
    auto* process = dynamic_cast<QProcess*>(QObject::sender());
    QString msg = process->readAllStandardOutput();
    msg.prepend(m_lastOutput);

    QStringList lines = msg.split("\n");
    if (!msg.endsWith("\n")) {
        m_lastOutput = lines.takeLast();
    }

    QString job = process->property("job").toString();
    for (const QString& line : asConst(lines)) {
        parseMsg(line);
        if (job == "scanDrives") {
            parseScanDrive(line);
        } else if (job == "info") {
            parseInfo(line);
        } else if (job == "importTrack" || job == "backupDisc") {
            parseProgress(line);
        }
    }
}

void MakeMkvCon::onReadyReadError()
{
    auto* process = dynamic_cast<QProcess*>(QObject::sender());
    qWarning() << process->readAllStandardError();
}

void MakeMkvCon::onFinished(int exitCode, QProcess::ExitStatus status)
{
    Q_UNUSED(exitCode);
    Q_UNUSED(status);
    auto* process = dynamic_cast<QProcess*>(QObject::sender());
    QString job = process->property("job").toString();

    if (job == "scanDrives") {
        emit sigGotDrives(m_drives);
    } else if (job == "info") {
        emit sigScannedDrive(m_title, m_tracks);
    } else if (job == "importTrack") {
        emit sigTrackImported(process->property("trackId").toInt());
    } else if (job == "backupDisc") {
        emit sigDiscBackedUp();
    }
}

void MakeMkvCon::parseMsg(QString line)
{
    QRegularExpression rx("MSG:(\\d+),\\d+,\\d+,\"(.*)\",\".*\"", QRegularExpression::InvertedGreedinessOption);
    QRegularExpressionMatch match = rx.match(line);
    if (match.hasMatch() && match.captured(1).toInt() != 5010) {
        emit sigMessage(match.captured(2).replace("\\\"", "\""));
    }
}

void MakeMkvCon::parseScanDrive(QString line)
{
    QRegularExpression rx(R"lit(DRV:(\d+),\d+,\d+,\d+,"(.*)","(.*)",".*")lit", //
        QRegularExpression::InvertedGreedinessOption);
    QRegularExpressionMatch match = rx.match(line);
    if (match.hasMatch() && !match.captured(2).isEmpty()) {
        m_drives.insert(match.captured(1).toInt(), QString("%1 (%2)").arg(match.captured(2)).arg(match.captured(3)));
    }
}

void MakeMkvCon::parseInfo(QString line)
{
    QRegularExpression rx("TINFO:(\\d+),(\\d+),\\d+,\"(.*)\"", QRegularExpression::InvertedGreedinessOption);
    QRegularExpressionMatch match = rx.match(line);
    if (match.hasMatch()) {
        int trackId = match.captured(1).toInt();
        int typeId = match.captured(2).toInt();
        QString value = match.captured(3);
        if (!m_tracks.contains(trackId)) {
            Track t;
            m_tracks.insert(trackId, t);
        }
        switch (typeId) {
        case 2: m_tracks[trackId].name = value; break;
        case 8: m_tracks[trackId].chapters = value.toInt(); break;
        case 9: m_tracks[trackId].duration = value; break;
        case 11: m_tracks[trackId].size = value.toLongLong(); break;
        case 27: m_tracks[trackId].fileName = value; break;
        default: break;
        }
    }

    rx.setPattern("CINFO:(\\d+),\\d+,\"(.*)\"");
    match = rx.match(line);
    if (match.hasMatch() && match.captured(1).toInt() == 2) {
        m_title = match.captured(2);
    }
}

void MakeMkvCon::parseProgress(QString line)
{
    QRegularExpression rx(R"(^PRGV:(\d+),(\d+),(\d+)$)", QRegularExpression::InvertedGreedinessOption);
    QRegularExpressionMatch match = rx.match(line);
    if (match.hasMatch()) {
        emit sigProgress(match.captured(2).toInt(), match.captured(3).toInt());
    }
}
