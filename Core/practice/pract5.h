#ifndef PRACT5_H
#define PRACT5_H

#include <QWidget>

namespace Ui {
class Pract5;
}

class Pract5 : public QWidget
{
    Q_OBJECT

public:
    explicit Pract5(QWidget *parent = nullptr);
    ~Pract5();

private:
    Ui::Pract5 *ui;
};

#endif // PRACT5_H
