#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{

    QStringList paths = QCoreApplication::libraryPaths();
    paths.append(".");
    paths.append("imageformats");
    paths.append("platforms");
    paths.append("sqldrivers");
    QCoreApplication::setLibraryPaths(paths);



    qRegisterMetaType<QByteArray>("QByteArray&");
    qRegisterMetaType<QString>("QString&");
    QApplication a(argc, argv);
    MainWindow *w= new MainWindow();
     w->setAttribute(Qt::WA_DeleteOnClose);
     w->show();



    return a.exec();
}
