#include "icontray.h"
#include "item_list.h"
#include "mainwindow.h"
#include <QApplication>
#include <QStyleFactory>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QApplication::setStyle(QStyleFactory::create("Fusion"));


    //MainWindow f;
    //f.show();
    Icontray w;
    w.icontray();

    return a.exec();
}
