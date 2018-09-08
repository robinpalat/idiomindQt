#ifndef PRACT3_H
#define PRACT3_H

#include <QWidget>

namespace Ui {
class Pract3;
}

class Pract3 : public QWidget
{
    Q_OBJECT

public:
    explicit Pract3(QWidget *parent = nullptr);
    ~Pract3();

private:
    Ui::Pract3 *ui;
};

#endif // PRACT3_H
