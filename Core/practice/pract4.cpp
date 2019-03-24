
#include "Core/practice/pract4.h"
#include "ui_pract4.h"
#include "dlg_practice.h"

#include <QTimer>


#include "Core/vars_statics.h"


Pract4::Pract4(QWidget *parent) : QWidget(parent), ui(new Ui::Pract4) {

    ui->setupUi(this);
    QSettings settings(ivar::FILE_conf, QSettings::IniFormat);
    restoreGeometry(settings.value("dlgPract4").toByteArray());
    QFont font_trgt = ui->label_trgt->font();
    font_trgt.setPointSize(28);
    ui->label_trgt->setFont(font_trgt);
    player = new QMediaPlayer(this);
    ui->pushButton_no->setIcon(QIcon(ivar::DS+"/images/no.png"));
    ui->pushButton_no->setText(tr("I did not know it"));
    ui->pushButton_ok->setIcon(QIcon(ivar::DS+"/images/yes.png"));
    ui->pushButton_ok->setText(tr("I Knew it"));
}


Pract4::~Pract4() {
    delete ui;
}


void Pract4::load_data(std::map<QString, QString> &tmp_list_pair_words,
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

/*  (2) */
void Pract4::cuestion_card() {

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
void Pract4::set_text_cuestion_card(QString trgt) {

    QString userimg1=DM_tl+"/.share/images/"+trgt.toLower()+"-1.jpg";
    QString userimg2=DM_tl+"/"+tpc+"/images/"+trgt.toLower()+".jpg";
    if(QFileInfo(userimg1).exists()) {
        ui->image_trgt->setPixmap(QPixmap(userimg1));
    }
    else if (QFileInfo(userimg2).exists()) {
        ui->image_trgt->setPixmap(QPixmap(userimg2));

    }

    trgt = "...";
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
void Pract4::on_pushButton_ok_clicked() {

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


void Pract4::on_pushButton_no_clicked() {

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



void Pract4::closeEvent( QCloseEvent* event ) {

    QSettings settings(ivar::FILE_conf, QSettings::IniFormat);
    settings.setValue("dlgPract4", saveGeometry());
    if(this->isVisible()){
        event->ignore();
        this->hide();
     }
    qDebug() << list_easy.size();
    qDebug() << list_learning.size();
    qDebug() << list_difficult.size();
    Practice dlg;
    dlg.go_back_results(count_quiz, list_easy, list_learning,
                                list_difficult, "Pract4");
    dlg.setModal(true);
    dlg.exec();
}


void Pract4::on_label_trgt_clicked() {

    Audioplayer path;
    player->setMedia(QUrl::fromLocalFile(path.pathplay(trgt)));
    player->play();
}

void Pract4::on_pushButton_answer_clicked() {

    ui->label_trgt->setText(trgt);
    srce = list_pair_words[trgt];
    ui->label_srce->setText(srce);
    ui->pushButton_answer->hide();
}
