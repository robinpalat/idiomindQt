#include "Core/practice/pract4.h"
#include "ui_pract4.h"

Pract4::Pract4(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Pract4)
{
    ui->setupUi(this);
}

Pract4::~Pract4()
{
    delete ui;
}
