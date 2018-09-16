#include "Core/practice/pract2.h"
#include "ui_pract2.h"
#include "dlg_practice.h"

#include <iostream>
#include <string>
#include <algorithm>
#include <random>

Pract2::Pract2(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Pract2)
{
    ui->setupUi(this);

    QSettings settings(ivar::FILE_conf, QSettings::IniFormat);
    restoreGeometry(settings.value("dlgPract2").toByteArray());

    player = new QMediaPlayer(this);
    ui->pushButton_no->setIcon(QIcon(ivar::DS+"/images/no.png"));
    ui->pushButton_ok->setIcon(QIcon(ivar::DS+"/images/yes.png"));

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
    count_quiz = count_items;
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
            count_items = 0;
        }
    }

    if (count_round == 3 || count_items < 1) {

        this->close();
    }
    else {
        trgt = list_words[count_pos];
        set_text_cuestion_card(trgt);
    }
}


/* ------------------------------------------------ (3) */
void Pract2::set_text_cuestion_card(QString trgt) {

    QString tmp_trgt;
    QFont font_trgt = ui->label_trgt->font();
    font_trgt.setPointSize(28);
    ui->label_trgt->setFont(font_trgt);

    ui->label_trgt->setText(trgt);

    ui->tableWidget->setRowCount(0);

    list_shufle.clear();

    srce = list_pair_words[trgt];
    list_shufle.push_back(srce);

    unsigned tmp_count_pos = count_pos;
    QString tmp_srce;
    for (short unsigned n = 0; n < 7; n++) {
        tmp_count_pos++;
        if (tmp_count_pos == count_items) {
            tmp_count_pos = tmp_count_pos - 8;
        }
        tmp_srce = list_pair_words[list_words[tmp_count_pos]];
        list_shufle.push_back(tmp_srce);
    }

    for (int unsigned long l = 0; l < list_shufle.size(); l++) {
        int unsigned long r = l + rand() % (list_shufle.size() - l);
        swap(list_shufle[l], list_shufle[r]);
    }

    bool twist = true;
    for ( auto it = list_shufle.begin(); it != list_shufle.end(); ++it  ) {
        if (twist == true) {
            ui->tableWidget->insertRow(ui->tableWidget->rowCount());
            srce = *it;
            QTableWidgetItem * abocada = new QTableWidgetItem(srce);
            ui->tableWidget->setItem(ui->tableWidget->rowCount() -1, 0, abocada);
            twist = false;
        }
       else if (twist == false) {
            srce = *it;
            QTableWidgetItem * abocada2 = new QTableWidgetItem(srce);
            ui->tableWidget->setItem(ui->tableWidget->rowCount() -1, 1, abocada2);
            twist = true;
        }
    }
}


/* ------------------------------------------------ (1) */
void Pract2::on_pushButton_ok_clicked() { // si / next

    count_pos++;
    count_ok++;

        if (count_round == 1) {
            list_easy.push_back(trgt);
        }
        else if (count_round == 2) {
            list_learnt.push_back(trgt);
        }

     ui->pushButton_ok->setText(tr("I Knew it (")+QString::number(count_ok)+")");

    cuestion_card();
}


void Pract2::on_pushButton_no_clicked() { // no / next

    count_pos++;
    count_no++;

        if (count_round == 1) {
            list_learning.push_back(trgt);
        }
        else if (count_round == 2) {
            list_difficult.push_back(trgt);
        }


    ui->pushButton_no->setText(tr("I did not know it (")+QString::number(count_no)+")");
    cuestion_card();
}


void Pract2::closeEvent( QCloseEvent* event ) {

    QSettings settings(ivar::FILE_conf, QSettings::IniFormat);
    settings.setValue("dlgPract2", saveGeometry());

    if(this->isVisible()){
        event->ignore();
        this->hide();
     }

    Practice * mPractice;
    mPractice = new Practice(this);

    mPractice->go_back_results(count_quiz, list_easy, list_learning,
                                list_difficult, "Pract2");

    mPractice->show();
}


void Pract2::on_label_trgt_clicked() {

    Audioplayer path;
    player->setMedia(QUrl::fromLocalFile(path.pathplay(trgt)));
    player->play();

}
