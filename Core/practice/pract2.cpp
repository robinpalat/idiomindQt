#include "Core/practice/pract2.h"
#include "ui_pract2.h"

Pract2::Pract2(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Pract2)
{
    ui->setupUi(this);
}

Pract2::~Pract2()
{
    delete ui;
}
