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


void Practice::startt() {
//    conn.Opendb(DM_tl+"/"+tpc+"/.conf/tpcdb");
//    QSqlQuery qry;
//    QString img;
//    for( int n = 1; n < 6; n = n + 1 ) {
//        qry.prepare("SELECT * FROM Practice_icons");
//        if (qry.exec( )) {
//            while(qry.next()) {
//                pair_practs["prac_a"]=qry.value(0).toString();
//                pair_practs["prac_b"]=qry.value(1).toString();
//                pair_practs["prac_c"]=qry.value(2).toString();
//                pair_practs["prac_d"]=qry.value(3).toString();
//                pair_practs["prac_e"]=qry.value(4).toString();
//            }
//        }
//    }
// qDebug() << pair_practs["prac_"+pracs_nicons[1]];
////qry.finish();
//conn.Closedb();
}

Practice::~Practice() {

    delete ui;
}

QString Practice::get_tpc() {
    Global mGlobal;
    return  mGlobal.get_textline(ivar::FILE_mn);
}

void Practice::score_info(unsigned long int total,
                          unsigned long int easy,
                          unsigned long int ling,
                          unsigned long int hard) {

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
    QString nicon, nicon_mod;

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


    pair_practs[active_pract]=nicon_mod;


    ui->widget_score->show();
    ui->label_score_total->setText(str_total);
    ui->label_score_learnt->setText(str_easy);
    ui->label_score_easy->setText(str_easy);
    ui->label_score_ling->setText(str_ling);
    ui->label_score_hard->setText(str_hard);
}

void Practice::load_data() {



    tpc = get_tpc();

    this->setWindowTitle(tr("Practice - ")+tpc);

    for( int n = 1; n < 6; n = n + 1 ) {

            ui->tableWidget->insertRow(ui->tableWidget->rowCount());

            QString imgPath = ivar::DS+"/images/practice/"+imgs[n]+".png";
            QImage *img = new QImage(imgPath);
            QTableWidgetItem *thumbnail = new QTableWidgetItem;
            thumbnail->setData(Qt::DecorationRole, QPixmap::fromImage(*img));
            ui->tableWidget->setItem(ui->tableWidget->rowCount() -1, 0, thumbnail);

            ui->tableWidget->setItem(ui->tableWidget->rowCount()-1 , 1, new QTableWidgetItem(pracs[n-1]));
        }


}


void Practice::on_pushButton_clicked() {



}

void Practice::on_tableWidget_itemDoubleClicked(QTableWidgetItem *item) {

    this->hide();

    active_pract = "pract_a";

    QSqlDatabase db = Database::instance().getConnection(DM_tl+"/"+tpc+"/.conf/tpcdb");
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

    if (active_pract=="pract_a") {

        mPrac_a = new Prac_a();
        mPrac_a->load_data(pair_words,words);
        mPrac_a->show();
    }
}


void Practice::save_data() {

    // ------------------------------icons stats
    unsigned long int isok = 100*int_easy/total;
    unsigned long int n = 1,  c = 1;

    QSqlDatabase db = Database::instance().getConnection(DM_tl+"/"+tpc+"/.conf/tpcdb");
    QSqlQuery qry(db);

    QString nicon, nicon_mod;
    qry.prepare("SELECT prac_a FROM Practice_icons");
    if (!qry.exec()) qDebug() << qry.lastError().text();
    while(qry.next()) { nicon = qry.value(0).toString(); }

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

    qry.prepare("UPDATE Practice_icons SET prac_a='"+nicon_mod+"' WHERE prac_a='"+nicon+"'");
    if (!qry.exec()) qDebug() << qry.lastError().text();

    qry.finish();
}

void Practice::closeEvent(QCloseEvent * event)
{
    if(this->isVisible()){
        event->ignore();
        this->hide();
    }
    save_data();
}
