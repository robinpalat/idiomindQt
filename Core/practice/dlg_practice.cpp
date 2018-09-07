#include <QCloseEvent>

#include "dlg_practice.h"
#include "build/ui/ui_dlg_practice.h"
#include "Core/vars_statics.h"
#include "Core/vars_session.h"

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

    startt();
    load_data();
}



Practice::~Practice() {

    delete ui;
}

QString Practice::get_tpc() {
    Global mGlobal;
    return mGlobal.get_textline(ivar::FILE_mn);
}

void Practice::score_info(unsigned long int total,
                          unsigned long int easy,
                          unsigned long int ling,
                          unsigned long int hard,
                          QString active_pract) {

    active_pract = active_pract;
    int_total = total;
    int_easy = easy;
    int_ling = ling;
    int_hard = hard;
    QString str_total =  QString::number(total);
    QString str_easy =  QString::number(easy);
    QString str_ling = QString::number(ling);
    //str_learnt = QString::number(learnt.size());
    QString str_hard = QString::number(hard);


    unsigned long int isok = 100*int_easy/total;
    unsigned long int n = 1,  c = 1;


    while ( n <= 21 ) {
        if ( n == 21 ) {
            nicon_mod = QString::number(n-1);
            break;
        }
        else if ( isok <= c ) {
            nicon_mod = QString::number(n);
            break;
        }
        c = c + 5;
        n++;
    }

    img_pair_practs[active_pract]=nicon_mod;

    ui->widget_score->show();
    ui->label_score_total->setText(str_total);
    ui->label_score_learnt->setText(str_easy);
    ui->label_score_easy->setText(str_easy);
    ui->label_score_ling->setText(str_ling);
    ui->label_score_hard->setText(str_hard);

    QSqlDatabase db = Database::instance().getConnection(tpc);
    QSqlQuery qry(db);

    qry.prepare("UPDATE Practice_icons SET "+active_pract+"='"+nicon_mod+"'");
    if (!qry.exec()) qDebug() << qry.lastError().text();
    qry.finish();

    load_data();
}

void Practice::startt() {

    QSqlDatabase db = Database::instance().getConnection(tpc);
    QSqlQuery qry(db);

    for( int n = 1; n < 6; n = n + 1 ) {
        qry.prepare("SELECT * FROM Practice_icons");
        if (qry.exec( )) {
            while(qry.next()) {
                img_pair_practs["pract1"]=qry.value(0).toString();
                img_pair_practs["pract2"]=qry.value(1).toString();
                img_pair_practs["pract3"]=qry.value(2).toString();
                img_pair_practs["pract4"]=qry.value(3).toString();
                img_pair_practs["pract5"]=qry.value(4).toString();
            }
        }
    }
    qry.finish();
}



void Practice::load_data() {

    tpc = get_tpc();



    this->setWindowTitle(tr("Practice - ")+tpc);


    ui->tableWidget->clearContents();
    ui->tableWidget->setRowCount(0);

    short unsigned n = 1;
    for ( auto it = img_pair_practs.begin(); it != img_pair_practs.end(); ++it  )
    {
       ui->tableWidget->insertRow(ui->tableWidget->rowCount());

       QString imgPath = ivar::DS+"/images/practice/"+it->second+".png";
       QImage *img = new QImage(imgPath);
       QTableWidgetItem *thumbnail = new QTableWidgetItem;
       thumbnail->setData(Qt::DecorationRole, QPixmap::fromImage(*img));

       ui->tableWidget->setItem(ui->tableWidget->rowCount() -1, 0, thumbnail);

       ui->tableWidget->setItem(ui->tableWidget->rowCount()-1 , 1, new QTableWidgetItem(practs[n-1]));

       n++;
    }
}


void Practice::on_pushButton_clicked() {



}

void Practice::on_tableWidget_itemDoubleClicked(QTableWidgetItem *item) { // item->text();

    this->hide();

    active_pract = "pract"+QString::number(item->row()+1);
    // qDebug() << active_pract;

    QSqlDatabase db = Database::instance().getConnection(tpc);
    QSqlQuery qry(db);

    qry.prepare("select list from learning");

    QString trgt, srce, type;

    total = 0;
    if (qry.exec( )) {

        while(qry.next()) {
            trgt = qry.value(0).toString();

            QSqlQuery qry(db);
            qry.prepare("select * from "+Source_LANG+" where trgt=(:trgt_val)");
            qry.bindValue(":trgt_val", trgt);

            if (qry.exec( )) {
                while(qry.next()) {
                    type = "2";
                    srce = qry.value(1).toString();
                    type = qry.value(13).toString();
                }
            }
            if (trgt != "" && srce != "" && type == "1") {
                words.push_back(trgt);
                pair_words[trgt]=srce;
                total ++;
            }
        }
    }

    qry.finish();

    if (active_pract == "pract1") {

        mPrac_a = new Prac_a();
        mPrac_a->load_data(pair_words,words);
        mPrac_a->show();
    }
}


void Practice::save_data() {

    // ------------------------------icons stats

}

void Practice::closeEvent(QCloseEvent * event)
{
    if(this->isVisible()){
        event->ignore();
        this->hide();
    }
    save_data();
}
