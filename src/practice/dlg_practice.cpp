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

void Practice::score_info(QString total, QString easy, QString ling, QString hard) {

    ui->widget_score->show();
    ui->label_score_total->setText(total);
    ui->label_score_learnt->setText(easy);
    ui->label_score_easy->setText(easy);
    ui->label_score_ling->setText(ling);
    ui->label_score_hard->setText(hard);

}

void Practice::load_data() {

    tpc = get_tpc();

    this->setWindowTitle(tr("Practice - ")+tpc);

    QString pracs[5] = {tr("Flashcards"),tr("Multiple-choice"),
                        tr("Recognize Pronunciation"),
                        tr("Images"),tr("Listen and Writing Sentences")};
    QString pracs_nicons[5] = {"a","b","c","d","e"};

    Database conn;
    conn.Opendb(DM_tl+"/"+tpc+"/.conf/tpcdb");
    QSqlQuery qry;

    QString img;
    for( int n = 1; n < 6; n = n + 1 ) {

            ui->tableWidget->insertRow(ui->tableWidget->rowCount());

            qry.prepare("SELECT prac_"+pracs_nicons[n-1]+" FROM Practice_icons");
            if (qry.exec( )) {
                while(qry.next()) {img = qry.value(0).toString();}
            }
            QString imgPath = ivar::DS+"/images/practice/"+img+".png";
            QImage *img = new QImage(imgPath);
            QTableWidgetItem *thumbnail = new QTableWidgetItem;
            thumbnail->setData(Qt::DecorationRole, QPixmap::fromImage(*img));
            ui->tableWidget->setItem(ui->tableWidget->rowCount() -1, 0, thumbnail);

            ui->tableWidget->setItem(ui->tableWidget->rowCount()-1 , 1, new QTableWidgetItem(pracs[n-1]));
        }

    conn.Closedb();
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
