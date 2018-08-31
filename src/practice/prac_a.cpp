#include "prac_a.h"
#include "ui_prac_a.h"
#include "dlg_practice.h"

Prac_a::Prac_a(QWidget *parent) : QWidget(parent), ui(new Ui::Prac_a) {

    ui->setupUi(this);

    ui->pushButton_no->setIcon(QIcon(ivar::DS+"/images/no.png"));
    ui->pushButton_no->setText(tr("I did not know it"));
    ui->pushButton_ok->setIcon(QIcon(ivar::DS+"/images/yes.png"));
    ui->pushButton_ok->setText(tr("I Knew it"));
}


Prac_a::~Prac_a() {
    delete ui;
}


void Prac_a::load_data(QString tpc) {

    tpc = tpc;

    Database conn;
    conn.Opendb(DM_tl+"/"+tpc+"/.conf/tpcdb");

    QSqlQuery qry_a;
    qry_a.prepare("select list from learning");

    QSqlQuery qry;

    total = 0;
    if (qry_a.exec( )) {

        while(qry_a.next()) {
            trgt = qry_a.value(0).toString();

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

    qry_a.finish();
    qry.finish();
    conn.Closedb();

    ok_count = 0;
    no_count = 0;
    items = total;
    pos = 0;
    round = 1;
    cuestion_card();
}


/* ------------------------------------------------ (3) */
void Prac_a::setLabelText_cuest(QString trgt) {

    QFont font_trgt = ui->label_trgt->font();
    font_trgt.setPointSize(28);
    ui->label_trgt->setFont(font_trgt);

    QFont font_srce = ui->label_srce->font();
    font_srce.setPointSize(16);
    ui->label_srce->setFont(font_srce);

    ui->label_trgt->setText(trgt);
    ui->label_srce->setText("");

    ui->pushButton_answer->show();
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

    ui->pushButton_answer->hide();
}

/* ------------------------------------------------ (1) */
void Prac_a::on_pushButton_ok_clicked() { // si / next

    if (cuest == false) pos++;

        if (round == 1) {
            easy.push_back(trgt);
        }
        else if (round == 2) {
            learnt.push_back(trgt);
        }

     ok_count++;
     ui->pushButton_ok->setText(tr("I Knew it (")+QString::number(ok_count)+")");

    cuestion_card();
}


void Prac_a::on_pushButton_no_clicked() { // no / next

    if (cuest == false ) pos++;

        if (round == 1) {
            learning.push_back(trgt);
        }
        else if (round == 2) {
            difficult.push_back(trgt);
        }
    no_count++;

    ui->pushButton_no->setText(tr("I did not know it (")+QString::number(no_count)+")");
    cuestion_card();
}


void Prac_a::on_pushButton_answer_clicked()
{
    answer_card();
}


/* ------------------------------------------------ (2) */
void Prac_a::cuestion_card() {

    if (pos == items) {
        if (round == 1) {
            round=2;
            pos=0;
            words.clear();
            words = learning;
            items = words.size();
        }
        else if (round == 2) {
            round=3;
            pos=0;
            words.clear();
            words = difficult;
            items = words.size();
        }
    }

    if (round == 3 || items < 1) {

        this->close();
    }
    else {
        trgt = words[pos];
        cuest = false;
        setLabelText_cuest(trgt);
    }

}

void Prac_a::answer_card() {

    trgt = words[pos];
    cuest = true;
    setLabelText_answer(trgt);
    pos++;

}


void Prac_a::save_data() {

    Database conn;
    conn.Opendb(DM_tl+"/"+tpc+"/.conf/tpcdb");

    // ----------------------------------------------- icons stats
    unsigned long int isok = 100*easy.size()/total;
    unsigned long int n = 1,  c = 1;

    QSqlQuery qry;
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


    conn.Closedb();
}


void Prac_a::closeEvent( QCloseEvent* event )
{
    save_data();

    if(this->isVisible()){
        event->ignore();
        this->hide();
     }

    scr_total =  QString::number(total);
    scr_easy =  QString::number(easy.size());
    scr_ling = QString::number(learning.size());
    scr_learnt = QString::number(learnt.size());
    scr_hard = QString::number(difficult.size());

    Practice * mPractice;
    mPractice = new Practice(this);
    if(items < 1){
        mPractice->score_info(scr_total, scr_easy, scr_ling, scr_hard);
    }
    else{
        mPractice->score_info(scr_total, scr_easy, scr_ling, scr_hard);
    }
    mPractice->show();
}


