#include "Core/practice/pract5.h"
#include "ui_pract5.h"

Pract5::Pract5(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Pract5)
{
    ui->setupUi(this);
}

Pract5::~Pract5()
{
    delete ui;
}
