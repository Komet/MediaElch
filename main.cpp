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
#endif

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QTranslator qtTranslator;
    qtTranslator.load(":/i18n/qt_" + QLocale::system().name());
    a.installTranslator(&qtTranslator);

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
    }

    MainWindow w;
    w.show();
    int ret = a.exec();

    if (data.isOpen())
        data.close();

    return ret;
}
