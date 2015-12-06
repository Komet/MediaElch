#include <QFile>
#include <QLibraryInfo>
#include <QMessageBox>
#include <QObject>
#include <QApplication>
#include <QFontDatabase>
#include <QTextCodec>
#include <QTextStream>
#include <QTimer>
#include <QTranslator>
#include "main/MainWindow.h"
#include "settings/Settings.h"

static QFile data;

void messageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    bool toFile = data.isOpen();

    QByteArray localMsg = msg.toLocal8Bit();
    QTextStream out;
    if (toFile)
        out.setDevice(&data);
    QString newLine = "\n";
    #ifdef Q_OS_WIN32
        newLine = "\r\n";
    #endif

    QString f = QString("%1").arg(context.function, -70, QChar(' '));

    switch (type) {
    case QtDebugMsg:
        if (toFile)
            out << "[" << f << "] " << localMsg << newLine;
        else
            fprintf(stderr, "%s %s%s", qPrintable(f), qPrintable(localMsg), qPrintable(newLine));
        break;
    case QtWarningMsg:
        if (toFile)
            out << "[" << f << "] " << "WARNING: " << localMsg << newLine;
        else
            fprintf(stderr, "%s WARNING: %s%s", qPrintable(f), qPrintable(localMsg), qPrintable(newLine));
        break;
    case QtCriticalMsg:
        if (toFile)
            out << "[" << f << "] " << "CRITICAL: " << localMsg << newLine;
        else
            fprintf(stderr, "%s CRITICAL: %s%s", qPrintable(f), qPrintable(localMsg), qPrintable(newLine));
        break;
    case QtFatalMsg:
        if (toFile)
            out << "[" << f << "] " << "FATAL: " << localMsg << newLine;
        else
            fprintf(stderr, "%s FATAL: %s%s", qPrintable(f), qPrintable(localMsg), qPrintable(newLine));
        abort();
    }
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // Qt localization
    QTranslator qtTranslator;
    qtTranslator.load(":/i18n/qt_" + QLocale::system().name());
    a.installTranslator(&qtTranslator);

    // MediaElch localization
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

    QCoreApplication::setOrganizationName("kvibes");
    QCoreApplication::setApplicationName("MediaElch");
    QCoreApplication::setApplicationVersion("2.4");
#if (QT_VERSION >= QT_VERSION_CHECK(5, 1, 0))
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps, true);
#endif

    Settings::instance(qApp)->loadSettings();
    if (Settings::instance()->advanced()->debugLog() && !Settings::instance()->advanced()->logFile().isEmpty()) {
        data.setFileName(Settings::instance()->advanced()->logFile());
        if (!data.open(QFile::WriteOnly | QFile::Truncate))
            QMessageBox::critical(0, QObject::tr("Logfile could not be openened"),
                                  QObject::tr("The logfile %1 could not be openend for writing.").arg(Settings::instance()->advanced()->logFile()));
    }
    qInstallMessageHandler(messageOutput);

    MainWindow w;
    w.show();
    int ret = a.exec();

    if (data.isOpen())
        data.close();

    return ret;
}
