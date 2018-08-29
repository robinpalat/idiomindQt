#include "src/dlg_practice.h"
#include "ui_dlg_practice.h"
#include "vars_statics.h"
#include "vars_session.h"

Practice::Practice(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Practice)
{
    ui->setupUi(this);
     load_data();
    ui->widget_final->hide();
    ui->widget_score->hide();
    ui->tableWidget->setColumnCount(2);
    ui->tableWidget->setRowCount(0);

    ui->tableWidget->setColumnWidth(0, 42);

    ui->tableWidget->setColumnWidth(1, 485);
    ui->tableWidget->setStyleSheet("QTableWidget::item { padding: 1px }");
    ui->tableWidget->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    ui->tableWidget->setItem(ui->tableWidget->rowCount() , 1, new QTableWidgetItem("tpcgggg"));

}

Practice::~Practice()
{
    delete ui;
}


void Practice::load_data() {

    Global mGlobal;
    for (int unsigned n = 1; n < 6 ; n++) {


         ui->tableWidget->insertRow(ui->tableWidget->rowCount());


            QString Path = DC_tlt+"/practice/."+QString::number(n);
            QString IMG = mGlobal.get_textline(Path);
            QString imgPath = ivar::DS+"/practice/images/"+IMG+".png";
            QImage *img = new QImage(imgPath);
            QTableWidgetItem *thumbnail = new QTableWidgetItem;
            thumbnail->setData(Qt::DecorationRole, QPixmap::fromImage(*img));
            ui->tableWidget->setItem(ui->tableWidget->rowCount() -1, 0, thumbnail);

            ui->tableWidget->setItem(ui->tableWidget->rowCount()-1 , 1, new QTableWidgetItem("tpcgggg"));


        }
}
