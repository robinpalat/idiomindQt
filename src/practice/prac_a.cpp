#include "prac_a.h"
#include "ui_prac_a.h"
#include "dlg_practice.h"

Prac_a::Prac_a(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Prac_a) {
    ui->setupUi(this);

    ui->pushButton_exit->setIcon(QIcon(ivar::DS+"/images/no.png"));
    ui->pushButton_nex->setText(tr("I did not know it"));
    ui->pushButton_exit->setText(tr("I Knew it"));

    cuest = true;

    //load_data();
}


Prac_a::~Prac_a() {
    delete ui;
}


void Prac_a::load_data(QString tpc) {

    tpc = tpc;
    QSqlDatabase mydb;
    mydb=QSqlDatabase::addDatabase("QSQLITE");
    mydb.setDatabaseName(DM_tl+"/"+tpc+"/.conf/tpcdb");
    if (!mydb.open()) qDebug()<<("Failed to open the database");

    QSqlQuery* qry_a=new QSqlQuery(mydb);
    qry_a->prepare("select list from learning");

    QSqlQuery qry;

    items = 0;
    if (qry_a->exec( )) {

        while(qry_a->next()) {
            trgt = qry_a->value(0).toString();

            qry.prepare("select * from "+Source_LANG+" where trgt=(:trgt_val)");
            qry.bindValue(":trgt_val", trgt);

            if (qry.exec( )) {
                while(qry.next()) {
                    type = "2";
                    srce = qry.value(0).toString();
                    type = qry.value(13).toString();
                }
            }
            if (trgt!="" && srce != "" && type == "1") {
                words.push_back(trgt);
                pair_words[trgt]=srce;
            }
            items ++;
        }
    }

    pos = 0;
    round = 1;
    cuest = true;
    on_pushButton_nex_clicked();

    qry_a->finish();
    qry.finish();
    mydb.close();
    mydb = QSqlDatabase();
    mydb.removeDatabase(QSqlDatabase::defaultConnection);
}


void Prac_a::setLabelText_cuest(QString trgt) {
    QFont font_trgt = ui->label_trgt->font();
    font_trgt.setPointSize(28);
    ui->label_trgt->setFont(font_trgt);

    QFont font_srce = ui->label_srce->font();
    font_srce.setPointSize(16);
    ui->label_srce->setFont(font_srce);

    ui->label_trgt->setText(trgt);
    ui->label_srce->setText("");

    ui->pushButton_nex->setIcon(QIcon::fromTheme(""));
    ui->pushButton_exit->hide();
    ui->pushButton_nex->setText(tr("Show Answer"));
}

void Prac_a::setLabelText_answer(QString trgt) {

    srce = pair_words[trgt];

    QFont font_trgt = ui->label_trgt->font();
    font_trgt.setPointSize(16);
    ui->label_trgt->setFont(font_trgt);

    QFont font_srce = ui->label_srce->font();
    font_srce.setPointSize(28);
    ui->label_srce->setFont(font_srce);

    ui->label_trgt->setText(trgt);
    ui->label_srce->setText(srce);

    ui->pushButton_exit->show();
    ui->pushButton_nex->setIcon(QIcon(ivar::DS+"/images/yes.png"));
    ui->pushButton_exit->setIcon(QIcon(ivar::DS+"/images/no.png"));
    ui->pushButton_nex->setText(tr("I Knew it"));
    ui->pushButton_exit->setText(tr("I did not know it"));

}


void Prac_a::on_pushButton_nex_clicked() { // si / next
    if (cuest == true) {
        if (round == 1) {
            easy.push_back(trgt);
        }
        else if (round == 2) {
            learnt.push_back(trgt);
        }
        cuestion_card();
    }
    else if (cuest == false) {
         answer_card();
    }
}


void Prac_a::on_pushButton_exit_clicked() { // no / next
    if (cuest == true) {
        if (round == 1) {
            learning.push_back(trgt);
        }
        else if (round == 2) {
            difficult.push_back(trgt);
        }
        cuestion_card();
    }
    else if (cuest == false) {
        this->close();
    }
}


void Prac_a::answer_card() {

        trgt = words[pos];
        cuest = true;
        setLabelText_answer(trgt);
        pos++;

    if ((pos)==items) {
        if (round == 1) {
            round=2; pos=0;
            words.clear();
            words = learning;
            items = words.size();

            qDebug() << items;
        }
        else if (round == 2) {
            round=3; pos=0;
            words.clear();
            words = difficult;
            items = words.size();
        }
    }
}


void Prac_a::cuestion_card() {

    if (round == 3) {
        qDebug() << easy.size();
        qDebug() << learning.size();
        qDebug() << learnt.size();
        qDebug() << difficult.size();
        this->close();
    }
    else {
        trgt = words[pos];
        cuest = false;
        setLabelText_cuest(trgt);
    }

}


void Prac_a::save_data() {


}


void Prac_a::closeEvent( QCloseEvent* event )
{
    //save_data();
    if(this->isVisible()){
        event->ignore();
        this->hide();
     }
    Practice * mPractice;
    mPractice = new Practice(this);
    mPractice->show();
}

