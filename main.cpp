#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    setlocale(0,"RUS");
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
