#include "imports/Extractor.h"

#include "settings/Settings.h"

#include <QDebug>
#include <QFileInfo>
#include <QProcess>
#include <QRegularExpression>

Extractor::Extractor(QObject* parent) : QObject(parent)
{
}

Extractor::~Extractor()
{
    for (QProcess* process : asConst(m_processes)) {
        process->kill();
    }
}

void Extractor::extract(QString baseName, QStringList files, QString password)
{
    QStringList rarFiles;
    for (const QString& file : files) {
        if (file.endsWith(".rar")) {
            rarFiles.append(file);
        }
    }
    if (rarFiles.isEmpty()) {
        emit sigError(baseName, tr("No files to extract"));
        emit sigFinished(baseName, false);
        return;
    }

    std::sort(rarFiles.begin(), rarFiles.end());

    QString unrar = Settings::instance()->importSettings().unrar();

    if (!QFileInfo(unrar).isFile()) {
        emit sigError(baseName, tr("Unrar not found"));
        emit sigFinished(baseName, false);
        return;
    }

    QString file = rarFiles.first();
    QFileInfo fi(file);

    QStringList parameters;
    parameters << "x"
               << "-o+"
               << "-y";
    if (!password.isEmpty()) {
        parameters << "-p" + password;
    }
    parameters << file;
    // parameters << fi.path();

    auto* process = new QProcess(this);
    m_processes.append(process);
    connect(process, &QProcess::readyReadStandardOutput, this, &Extractor::onReadyRead);
    connect(process, &QProcess::readyReadStandardError, this, &Extractor::onReadyReadError);
    connect(process, elchOverload<int, QProcess::ExitStatus>(&QProcess::finished), this, &Extractor::onFinished);
    process->setProperty("baseName", baseName);
    process->setProperty("hasError", false);
    process->setWorkingDirectory(fi.path());
    process->start(unrar, parameters);
    process->waitForStarted(10000);
}

void Extractor::onReadyRead()
{
    auto* process = dynamic_cast<QProcess*>(QObject::sender());
    QString msg = process->readAllStandardOutput();
    QRegularExpression rx("([0-9]*)%");
    QRegularExpressionMatch match = rx.match(msg);
    if (match.hasMatch()) {
        emit sigProgress(process->property("baseName").toString(), match.captured(1).toInt());
    }
}

void Extractor::onReadyReadError()
{
    auto* process = dynamic_cast<QProcess*>(QObject::sender());
    QString msg = process->readAllStandardError();
    qDebug() << "ERROR" << msg;
    process->setProperty("hasError", true);
    process->kill();
    emit sigError(process->property("baseName").toString(), msg);
}

void Extractor::onFinished(int exitCode, QProcess::ExitStatus status)
{
    Q_UNUSED(exitCode);
    Q_UNUSED(status);
    auto* process = dynamic_cast<QProcess*>(QObject::sender());
    m_processes.removeAll(process);
    process->deleteLater();
    emit sigFinished(process->property("baseName").toString(), !process->property("hasError").toBool());
}

void Extractor::stopExtraction(QString baseName)
{
    for (QProcess* process : asConst(m_processes)) {
        if (process->property("baseName").toString() == baseName) {
            process->setProperty("hasError", true);
            process->kill();
            break;
        }
    }
}
