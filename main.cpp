#include <QtGui/QApplication>
#include <QTextCodec>
#include <QTranslator>
#include "MainWindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QTranslator editTranslator;
    QString filename;
    filename = QString("MediaElch_%1"). arg(QLocale::system().name());
    editTranslator.load(":/i18n/" + filename);
    a.installTranslator(&editTranslator);

    QTextCodec::setCodecForTr(QTextCodec::codecForName("UTF-8"));
    QCoreApplication::setOrganizationName("kVibes");
    QCoreApplication::setApplicationName("MediaElch");
    QCoreApplication::setApplicationVersion("0.9.1");

    MainWindow w;
    w.show();

    return a.exec();
}
