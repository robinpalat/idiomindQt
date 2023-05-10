
#include "Core/practice/pract3.h"
#include "ui_pract3.h"
#include "dlg_practice.h"

#include <QTimer>


#include "Core/vars_statics.h"


Pract3::Pract3(QWidget *parent) : QWidget(parent), ui(new Ui::Pract3) {

    ui->setupUi(this);
    QSettings settings(ivar::FILE_conf, QSettings::IniFormat);
    restoreGeometry(settings.value("dlgPract3").toByteArray());
    QFont font_trgt = ui->label_trgt->font();
    font_trgt.setPointSize(28);
    ui->label_trgt->setFont(font_trgt);
    player = new QMediaPlayer(this);
    ui->pushButton_no->setIcon(QIcon(ivar::DS+"/images/nou.png"));
    ui->pushButton_no->setText(tr("I did not know it"));
    ui->pushButton_ok->setIcon(QIcon(ivar::DS+"/images/yes.png"));
    ui->pushButton_ok->setText(tr("I Knew it"));
}


Pract3::~Pract3() {
    delete ui;
}


void Pract3::load_data(std::map<QString, QString> &tmp_list_pair_words,
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


/* (2) */
void Pract3::cuestion_card() {
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
            count_items = 0;
        }
    }
    if (count_round == 3 || count_items < 1) {
        this->close();
    }
    else {
        cuest = false;
        trgt = list_words[count_pos];
        set_text_cuestion_card(trgt);
    }
}


/* (3) */
void Pract3::set_text_cuestion_card(QString trgt) {

    QTimer::singleShot(1000, this, SLOT(on_label_trgt_clicked()));
    trgt = hide_word(trgt.toStdString());
    ui->label_trgt->setText(trgt);
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
    ui->label_srce->setText("");
    ui->pushButton_answer->show();
}


/* (1) */
void Pract3::on_pushButton_ok_clicked() {

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


void Pract3::on_pushButton_no_clicked() {

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



void Pract3::closeEvent( QCloseEvent* event ) {

    QSettings settings(ivar::FILE_conf, QSettings::IniFormat);
    settings.setValue("dlgPract3", saveGeometry());
    if(this->isVisible()){
        event->ignore();
        this->hide();
     }
    Practice dlg;
    qDebug() << list_easy.size();
    qDebug() << list_learning.size();
    qDebug() << list_difficult.size();
    dlg.go_back_results(count_quiz, list_easy, list_learning,
                                list_difficult, "Pract3");
    dlg.setModal(true);
    dlg.exec();
}


void Pract3::on_label_trgt_clicked() {
    Audioplayer path;
    player->setSource(QUrl::fromLocalFile(path.pathplay(trgt)));
    player->play();
}


void Pract3::on_pushButton_answer_clicked() {
    ui->label_trgt->setText(trgt);
    srce = list_pair_words[trgt];
    ui->label_srce->setText(srce);
    ui->pushButton_answer->hide();
}


void Pract3::on_pushButton_listen_clicked() {
    on_label_trgt_clicked();
}


QString Pract3::hide_word(const std::string& infix) {
    unsigned n = 0; QString trgt;
    infix.size();
    for(auto c : infix) {
        if ( n == 0) {
           trgt += c;
        }
        else if (n+1 == infix.size()) {
            trgt += " ";
            trgt += c;
        }
        else {
            trgt += " .";
        }
        n++;
    }
    return trgt;
}
