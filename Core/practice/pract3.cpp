#include "Core/practice/pract3.h"
#include "ui_pract3.h"

Pract3::Pract3(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Pract3)
{
    ui->setupUi(this);
}

Pract3::~Pract3()
{
    delete ui;
}
