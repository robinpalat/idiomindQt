#include "icontray.h"
#include "item_list.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    item_list f;
    f.load_data();
    Icontray w;
    w.icontray();

    return a.exec();
}
