#include "Core/practice/pract2.h"
#include "ui_pract2.h"
#include "dlg_practice.h"

#include <iostream>
#include <string>

Pract2::Pract2(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Pract2)
{
    ui->setupUi(this);

    player = new QMediaPlayer(this);

    ui->tableWidget->setColumnCount(2);
    ui->tableWidget->setColumnWidth(0, 285);
    ui->tableWidget->setColumnWidth(1, 285);

}

Pract2::~Pract2()
{
    delete ui;
}


void Pract2::load_data(std::map<QString, QString> &tmp_list_pair_words,
                       std::vector< QString > &tmp_list_words) {

    list_words = tmp_list_words;
    list_pair_words = tmp_list_pair_words;
    count_ok = 0;
    count_no = 0;
    count_items = list_words.size();
    count_total = count_items;
    count_pos = 0;
    count_round = 1;
    cuestion_card();
}

/* ------------------------------------------------ (2) */
void Pract2::cuestion_card() {

    if (count_pos == count_items) {
        if (count_round == 1) {
            count_round=2;
            count_pos=0;
            list_words.clear();
            list_words = list_learning;
            count_items = list_words.size();
        }
        else if (count_round == 2) {
            count_round=3;
            count_pos=0;
            list_words.clear();
            list_words = list_difficult;
            count_items = list_words.size();
        }
    }

    if (count_round == 3 || count_items < 1) {

        this->close();
    }
    else {
        trgt = list_words[count_pos];
        cuest = false;
        set_text_cuestion_card(trgt);
    }
}


/* ------------------------------------------------ (3) */
void Pract2::set_text_cuestion_card(QString trgt) {

    QFont font_trgt = ui->label_trgt->font();
    font_trgt.setPointSize(28);
    ui->label_trgt->setFont(font_trgt);

    ui->label_trgt->setText(trgt);

    ui->tableWidget->setRowCount(0);

    for (int short n = 0; n < 4 ; n++) {

        ui->tableWidget->insertRow(ui->tableWidget->rowCount());

        trgt = list_words[rand() % count_items];
        srce = list_pair_words[trgt];
        QTableWidgetItem * abocada = new QTableWidgetItem(srce);
        ui->tableWidget->setItem(ui->tableWidget->rowCount() -1, 0, abocada);

        trgt = list_words[rand() % count_items];
        srce = list_pair_words[trgt];
        QTableWidgetItem * abocada2 = new QTableWidgetItem(srce);
        ui->tableWidget->setItem(ui->tableWidget->rowCount() -1, 1, abocada2);
    }
}


/* ------------------------------------------------ (1) */
void Pract2::on_pushButton_ok_clicked() { // si / next

    if (cuest == false) count_pos++;

        if (count_round == 1) {
            list_easy.push_back(trgt);
        }
        else if (count_round == 2) {
            list_learnt.push_back(trgt);
        }

     count_ok++;
     ui->pushButton_ok->setText(tr("I Knew it (")+QString::number(count_ok)+")");

    cuestion_card();
}


void Pract2::on_pushButton_no_clicked() { // no / next

    if (cuest == false ) count_pos++;

        if (count_round == 1) {
            list_learning.push_back(trgt);
        }
        else if (count_round == 2) {
            list_difficult.push_back(trgt);
        }
    count_no++;

    ui->pushButton_no->setText(tr("I did not know it (")+QString::number(count_no)+")");
    cuestion_card();
}


void Pract2::closeEvent( QCloseEvent* event ) {

    QSettings settings(ivar::FILE_conf, QSettings::IniFormat);
    settings.setValue("dlgpract1", saveGeometry());

    if(this->isVisible()){
        event->ignore();
        this->hide();
     }

    Practice * mPractice;
    mPractice = new Practice(this);

    if(count_items < 1){
        mPractice->go_back_results(count_total, list_easy,
                                   list_learnt, list_learning,
                                   list_difficult, "pract1");
    }
    else{
        mPractice->go_back_results(count_total, list_easy,
                                   list_learnt, list_learning,
                                   list_difficult, "pract1");
    }
    mPractice->show();
}


void Pract2::on_label_trgt_clicked() {

    Audioplayer path;
    player->setMedia(QUrl::fromLocalFile(path.pathplay(trgt)));
    player->play();

}
