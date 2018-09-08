#ifndef PRACT4_H
#define PRACT4_H

#include <QWidget>

namespace Ui {
class Pract4;
}

class Pract4 : public QWidget
{
    Q_OBJECT

public:
    explicit Pract4(QWidget *parent = nullptr);
    ~Pract4();

private:
    Ui::Pract4 *ui;
};

#endif // PRACT4_H
