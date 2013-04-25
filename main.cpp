#include <QFile>
#include <QLibraryInfo>
#include <QMessageBox>
#include <QObject>
#include <QApplication>
#include <QTextCodec>
#include <QTextStream>
#include <QTimer>
#include <QTranslator>
#include "main/MainWindow.h"
#include "settings/Settings.h"
#include "cli/CLI.h"

static QFile data;

#if QT_VERSION < 0x050000

void messageOutput(QtMsgType type, const char *msg)
{
    if (!data.isOpen())
        return;

    QTextStream out(&data);
    QString newLine = "\n";
    #ifdef Q_OS_WIN32
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
    #ifdef Q_OS_WIN32
        newLine = "\r\n";
    #endif

    QString message(msg);
    message.append(newLine);

    switch (type) {
    case QtDebugMsg:
        fprintf(stderr, "DEBUG: %s", qPrintable(message));
        break;
    case QtWarningMsg:
        fprintf(stderr, "WARNING: %s", qPrintable(message));
        break;
    case QtCriticalMsg:
        fprintf(stderr, "CRITICAL: %s", qPrintable(message));
        break;
    case QtFatalMsg:
        fprintf(stderr, "FATAL: %s", qPrintable(message));
        abort();
    }
}

#else

void messageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    if (!data.isOpen())
        return;

    QByteArray localMsg = msg.toLocal8Bit();
    QTextStream out(&data);
    QString newLine = "\n";
    #ifdef Q_OS_WIN32
        newLine = "\r\n";
    #endif

    switch (type) {
    case QtDebugMsg:
        out << "[" << context.function << "] " << localMsg << newLine;
        break;
    case QtWarningMsg:
        out << "WARNING: " << "[" << context.function << "] " << localMsg << newLine;
        break;
    case QtCriticalMsg:
        out << "CRITICAL: " << "[" << context.function << "] " << localMsg << newLine;
        break;
    case QtFatalMsg:
        out << "FATAL: " << "[" << context.function << "] " << localMsg << newLine;
        abort();
    }
}

void cliMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QByteArray localMsg = msg.toLocal8Bit();

    switch (type) {
    case QtDebugMsg:
        fprintf(stderr, "DEBUG: [%s] %s\n", context.function, localMsg.constData());
        break;
    case QtWarningMsg:
        fprintf(stderr, "WARNING: [%s] %s\n", context.function, localMsg.constData());
        break;
    case QtCriticalMsg:
        fprintf(stderr, "CRITICAL: [%s] %s\n", context.function, localMsg.constData());
        break;
    case QtFatalMsg:
        fprintf(stderr, "FATAL: [%s] %s\n", context.function, localMsg.constData());
        abort();
    }
}

#endif

int main(int argc, char *argv[])
{
#ifdef Q_OS_LINUX
    bool useGui = getenv("DISPLAY") != 0;
#else
    bool useGui = true;
#endif

    QApplication a(argc, argv, useGui);

    QTranslator qtTranslator;
    qtTranslator.load(":/i18n/qt_" + QLocale::system().name());
    a.installTranslator(&qtTranslator);
    qDebug() << QLibraryInfo::location(QLibraryInfo::TranslationsPath);

    QTranslator editTranslator;
    QString filename;
    filename = QString("MediaElch_%1").arg(QLocale::system().name());

    QString localFileName = QString("%1%2MediaElch_local.qm").arg(QCoreApplication::applicationDirPath())
                                                          .arg(QDir::separator());
    QFileInfo fi(localFileName);
    if (fi.isFile())
        editTranslator.load(localFileName);
    else
        editTranslator.load(":/i18n/" + filename);
    a.installTranslator(&editTranslator);

#if QT_VERSION < 0x050000
    QTextCodec::setCodecForTr(QTextCodec::codecForName("UTF-8"));
#endif
    QCoreApplication::setOrganizationName("kvibes");
    QCoreApplication::setApplicationName("MediaElch");
    QCoreApplication::setApplicationVersion("1.7-dev");

    if (!useGui || a.arguments().count() > 1) {
#if QT_VERSION >= 0x050000
        qInstallMessageHandler(cliMessageOutput);
#else
        qInstallMsgHandler(cliMessageOutput);
#endif
        CLI *cli = new CLI(&a, a.arguments());
        QObject::connect(cli, SIGNAL(finished()), &a, SLOT(quit()));
        QTimer::singleShot(0, cli, SLOT(run()));
        return a.exec();
    }


    Settings::instance(qApp);
    if (Settings::instance()->advanced()->debugLog() && !Settings::instance()->advanced()->logFile().isEmpty()) {
        data.setFileName(Settings::instance()->advanced()->logFile());
        if (!data.open(QFile::WriteOnly | QFile::Truncate))
            QMessageBox::critical(0, QObject::tr("Logfile could not be openened"),
                                  QObject::tr("The logfile %1 could not be openend for writing.").arg(Settings::instance()->advanced()->logFile()));
#if QT_VERSION >= 0x050000
        qInstallMessageHandler(messageOutput);
#else
        qInstallMsgHandler(messageOutput);
#endif
    } else {
#if QT_VERSION >= 0x050000
        qInstallMessageHandler(cliMessageOutput);
#else
        qInstallMsgHandler(cliMessageOutput);
#endif
    }

    MainWindow w;
    w.show();
    int ret = a.exec();

    if (data.isOpen())
        data.close();

    return ret;
}
