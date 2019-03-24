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
    QFont font_trgt = ui->label_trgt->font();
    font_trgt.setPointSize(28);
    ui->label_trgt->setFont(font_trgt);
    ui->pushButton_no->setIcon(QIcon(ivar::DS+"/images/no.png"));
    ui->pushButton_ok->setIcon(QIcon(ivar::DS+"/images/yes.png"));
    ui->tableWidget->horizontalHeader()->setStretchLastSection(true);
    ui->tableWidget->setColumnCount(2);
    ui->tableWidget->setStyleSheet("QTableWidget::item { padding: 10px }");
    ui->tableWidget->setStyleSheet("QTableWidget::item { padding: 10px }");
    //ui->pushButton_ok->setEnabled(false);
}


Pract2::~Pract2() {
    delete ui;
}


void Pract2::load_data(std::map<QString, QString> &tmp_list_pair_words,
                       std::vector< QString > &tmp_list_words) {

    QString trgt, srce, type;
    QSqlDatabase db = Database::instance().getConnection(tpc);
    QSqlQuery qry(db);
    qry.prepare("SELECT list FROM words");
    if (qry.exec( )) {
        while(qry.next()) {
            trgt = qry.value(0).toString();
            QSqlQuery qry(db);
            qry.prepare("SELECT * FROM "+Source_LANG+" WHERE trgt=(:trgt_val)");
            qry.bindValue(":trgt_val", trgt);
            qry.exec( );
            qry.next();
            type = "2";
            srce = qry.value(1).toString();
            type = qry.value(13).toString();
            if (trgt != "" && srce != "" && type == "1") {
                list_pair_words_words[trgt]=srce;
                list_words_words.push_back(trgt);
            }
        }
    }

    list_words = tmp_list_words;
    list_pair_words = tmp_list_pair_words;
    list_easy.clear();
    list_learnt.clear();
    list_learning.clear();
    list_difficult.clear();
    count_ok = 0;
    count_no = 0;
    count_items = list_words.size();
    count_quiz = count_items;
    count_pos = 0;
    count_round = 1;
    cuestion_card();
}

/* (2) */
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
            list_words.clear();
            count_round=3;
            count_items = 0;
        }
    }
    if (count_round == 3 || count_items < 1) {
        list_words.clear();
        this->close();
    }
    else {
        trgt = list_words[count_pos];
        set_text_cuestion_card(trgt);
    }
}


/* (3) */
void Pract2::set_text_cuestion_card(QString trgt) {

    ui->label_trgt->setText(trgt);
    list_shufle.clear();
    list_shufle.push_back(list_pair_words[trgt]);
    tmp_count_pos = count_pos;
    QString tmp_srce, tmp_trgt;
    for (short unsigned n = 0; n < 7; n++) {
        tmp_count_pos++;
        if (tmp_count_pos >= count_items) {
            if(tmp_count_pos > 7) {
                tmp_count_pos = tmp_count_pos - 8;
            }
        }
        tmp_trgt = list_words_words[tmp_count_pos];
        tmp_srce = list_pair_words_words[tmp_trgt];
        list_shufle.push_back(tmp_srce);
    }
    for (int unsigned long l = 0; l < 8; l++) {
        int unsigned long r = l + rand() % (8 - l);
        swap(list_shufle[l], list_shufle[r]);
    }

    twist = true;
    ui->tableWidget->setRowCount(0);
    for ( auto it = list_shufle.begin(); it != list_shufle.end(); ++it  ) {
        if (twist == true) {
            ui->tableWidget->insertRow(ui->tableWidget->rowCount());
            QTableWidgetItem * a = new QTableWidgetItem(*it);
            ui->tableWidget->setItem(ui->tableWidget->rowCount() -1, 0, a);
            twist = false;
        }
       else if (twist == false) {
            QTableWidgetItem * b = new QTableWidgetItem(*it);
            ui->tableWidget->setItem(ui->tableWidget->rowCount() -1, 1, b);
            twist = true;
        }
    }
    QGraphicsOpacityEffect *effect = new QGraphicsOpacityEffect();
    ui->widget->setGraphicsEffect(effect);
    QPropertyAnimation *anim = new QPropertyAnimation(effect,"opacity");
    anim->setDuration(400);
    anim->setStartValue(0.0);
    anim->setEndValue(1.0);
    anim->setEasingCurve(QEasingCurve::OutQuad);
    connect(anim, &QPropertyAnimation::finished, [=]()
    {
        ui->widget->show();
    });
    anim->start(QAbstractAnimation::KeepWhenStopped);
}


/* (1) */
void Pract2::on_pushButton_ok_clicked() { // si / next

    if (srce_cell == list_pair_words[trgt]) {
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
    else {
        player->setMedia(QUrl::fromLocalFile(ivar::DS+"practice/no.mp3"));
        player->play();
        on_pushButton_no_clicked();
    }
    srce_cell = "";
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
    Practice dlg;
    dlg.go_back_results(count_quiz, list_easy, list_learning,
                                list_difficult, "Pract2");
    dlg.setModal(true);
    dlg.exec();
}


void Pract2::on_label_trgt_clicked() {

    Audioplayer path;
    player->setMedia(QUrl::fromLocalFile(path.pathplay(trgt)));
    player->play();
}


void Pract2::resizeEvent(QResizeEvent *event) {

    ui->tableWidget->setColumnWidth(0, ui->widget_2->width()/2-1);
    ui->tableWidget->setColumnWidth(1, ui->widget_2->width()/2-1);
    QHeaderView *verticalHeader = ui->tableWidget->verticalHeader();
    verticalHeader->setSectionResizeMode(QHeaderView::Fixed);
    verticalHeader->setDefaultSectionSize(ui->widget_2->height()/4);
}

void Pract2::on_tableWidget_cellClicked(int row, int column)
{
    srce_cell = ui->tableWidget->item(row, column)->text();
    on_pushButton_ok_clicked();
    //ui->pushButton_ok->setEnabled(true);
}

void Pract2::on_tableWidget_doubleClicked(const QModelIndex &index)
{
//    srce_cell = ui->tableWidget->item(index)->text();
//    ui->pushButton_ok->setEnabled(true);
}
