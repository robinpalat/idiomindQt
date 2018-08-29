#include <QCloseEvent>

#include "dlg_practice.h"
#include "ui_dlg_practice.h"
#include "../vars_statics.h"
#include "../vars_session.h"

Practice::Practice(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Practice)
{
    ui->setupUi(this);

    ui->widget_final->hide();
    ui->widget_score->hide();
    ui->tableWidget->setColumnCount(2);
    ui->tableWidget->setRowCount(0);
    ui->tableWidget->setColumnWidth(0, 68);
    ui->tableWidget->setColumnWidth(1, 335);
    QHeaderView *verticalHeader = ui->tableWidget->verticalHeader();
    verticalHeader->setSectionResizeMode(QHeaderView::Fixed);
    verticalHeader->setDefaultSectionSize(68);
    //ui->tableWidget->setStyleSheet("QTableWidget::item { margin-right: 20px }");

    load_data();

}

Practice::~Practice()
{
    delete ui;
}

QString Practice::get_tpc() {
    Global mGlobal;
    return  mGlobal.get_textline(ivar::FILE_mn);
}


void Practice::load_data() {

    tpc = get_tpc();

    this->setWindowTitle(tr("Practice - ")+tpc);

    Global mGlobal;
    QString pracs[5] = {tr("Flashcards"),tr("Multiple-choice"),
                        tr("Recognize Pronunciation"),
                        tr("Images"),tr("Listen and Writing Sentences")};

    for( int n = 1; n < 6; n = n + 1 ) {
            ui->tableWidget->insertRow(ui->tableWidget->rowCount());

            QString Path = DC_tlt+"/practice/."+QString::number(n);
            QString IMG = mGlobal.get_textline(Path);
            QString imgPath = ivar::DS+"/practice/images/"+IMG+".png";
            QImage *img = new QImage(imgPath);
            QTableWidgetItem *thumbnail = new QTableWidgetItem;
            thumbnail->setData(Qt::DecorationRole, QPixmap::fromImage(*img));
            ui->tableWidget->setItem(ui->tableWidget->rowCount() -1, 0, thumbnail);

            ui->tableWidget->setItem(ui->tableWidget->rowCount()-1 , 1, new QTableWidgetItem(pracs[n-1]));
        }
}


void Practice::closeEvent(QCloseEvent * event)
{
    if(this->isVisible()){
        event->ignore();
        this->hide();
    }
}

void Practice::on_pushButton_clicked()
{

}

void Practice::on_tableWidget_itemDoubleClicked(QTableWidgetItem *item)
{
    this->hide();
    mPrac_a = new Prac_a();

    mPrac_a->load_data(tpc);

    mPrac_a->show();
}
