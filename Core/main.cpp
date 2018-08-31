#include "icontray.h"
#include "item_list.h"
#include <QApplication>
#include <QStyleFactory>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QApplication::setStyle(QStyleFactory::create("Fusion"));


    item_list f;
    f.load_data();
    Icontray w;
    w.icontray();

    return a.exec();
}
