#include "icontray.h"
#include "mainwindow.h"
#include <QApplication>
#include <QStyleFactory>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QApplication::setStyle(QStyleFactory::create("Fusion"));

    Icontray w;
    w.icontray();

    return a.exec();
}
