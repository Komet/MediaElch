#include <QFile>
#include <QLibraryInfo>
#include <QMessageBox>
#include <QObject>
#include <QtGui/QApplication>
#include <QTextCodec>
#include <QTextStream>
#include <QTimer>
#include <QTranslator>
#include "main/MainWindow.h"
#include "settings/Settings.h"
#include "cli/CLI.h"

static QFile data;

void messageOutput(QtMsgType type, const char *msg)
{
    if (!data.isOpen())
        return;

    QTextStream out(&data);
    QString newLine = "\n";
    #ifdef Q_WS_WIN
        newLine = "\r\n";
    #endif

    switch (type) {
    case QtDebugMsg:
        out << msg << newLine;
        break;
    case QtWarningMsg:
        out << "WARNING: " << msg << newLine;
        break;
    case QtCriticalMsg:
        out << "CRITICAL: " << msg << newLine;
        break;
    case QtFatalMsg:
        out << "FATAL: " << msg << newLine;
        abort();
    }
}

void cliMessageOutput(QtMsgType type, const char *msg)
{
    QString newLine = "\n";
    #ifdef Q_WS_WIN
        newLine = "\r\n";
    #endif

    QString message(msg);
    message.append(newLine);

    switch (type) {
    case QtDebugMsg:
        // printf("DEBUG: %s", qPrintable(message));
        break;
    case QtWarningMsg:
        printf("WARNING: %s", qPrintable(message));
        break;
    case QtCriticalMsg:
        printf("CRITICAL: %s", qPrintable(message));
        break;
    case QtFatalMsg:
        printf("FATAL: %s", qPrintable(message));
        abort();
    }
}

int main(int argc, char *argv[])
{
#ifdef Q_WS_X11
    bool useGui = getenv("DISPLAY") != 0;
#else
    bool useGui = true;
#endif

    QApplication a(argc, argv, useGui);

    QTranslator qtTranslator;
    qtTranslator.load("qt_" + QLocale::system().name(), QLibraryInfo::location(QLibraryInfo::TranslationsPath));
    a.installTranslator(&qtTranslator);

    QTranslator editTranslator;
    QString filename;
    filename = QString("MediaElch_%1").arg(QLocale::system().name());

    QString localFileName = QString("%1%2MediaElch_%3.qm").arg(QCoreApplication::applicationDirPath())
                                                          .arg(QDir::separator())
                                                          .arg(QLocale::system().name().left(QLocale::system().name().indexOf("_")));
    QFileInfo fi(localFileName);
    if (fi.isFile())
        editTranslator.load(localFileName);
    else
        editTranslator.load(":/i18n/" + filename);
    a.installTranslator(&editTranslator);

    QTextCodec::setCodecForTr(QTextCodec::codecForName("UTF-8"));
    QCoreApplication::setOrganizationName("Daniel Kabel");
    QCoreApplication::setApplicationName("MediaElch");
    QCoreApplication::setApplicationVersion("1.2");

    if (!useGui || a.arguments().count() > 1) {
        qInstallMsgHandler(cliMessageOutput);
        CLI *cli = new CLI(&a, a.arguments());
        QObject::connect(cli, SIGNAL(finished()), &a, SLOT(quit()));
        QTimer::singleShot(0, cli, SLOT(run()));
        return a.exec();
    }

    QSettings settings;
    if (settings.value("DebugModeActivated", false).toBool() && !settings.value("DebugLogPath").toString().isEmpty()) {
        data.setFileName(settings.value("DebugLogPath").toString());
        if (!data.open(QFile::WriteOnly | QFile::Truncate))
            QMessageBox::critical(0, QObject::tr("Logfile could not be openened"),
                                  QObject::tr("The logfile %1 could not be openend for writing.").arg(settings.value("DebugLogPath").toString()));
        qInstallMsgHandler(messageOutput);
    }

    MainWindow w;
    w.show();
    int ret = a.exec();

    if (data.isOpen())
        data.close();

    return ret;
}
