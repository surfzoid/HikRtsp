#include "HikRtsp.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    qputenv("QT_ASSUME_STDERR_HAS_CONSOLE", "1");
    return a.exec();
}
