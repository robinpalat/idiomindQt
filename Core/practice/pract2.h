#ifndef PRACT2_H
#define PRACT2_H

#include <QWidget>

namespace Ui {
class Pract2;
}

class Pract2 : public QWidget
{
    Q_OBJECT

public:
    explicit Pract2(QWidget *parent = nullptr);
    ~Pract2();

private:
    Ui::Pract2 *ui;
};

#endif // PRACT2_H
