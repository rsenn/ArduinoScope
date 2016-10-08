#include "mainwindow.h"
#include <QApplication>
#include <QSettings>

int
main(int argc,
      char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    QSettings settings("ArduinoScope", "ArduinoScope");
    w.restoreGeometry(settings.value("window_geometry").toByteArray());
    w.restoreState(settings.value("window_state").toByteArray());
    w.show();

    return a.exec();
}
