#include "pract1.h"
#include "build/ui/ui_pract1.h"
#include "dlg_practice.h"
#include "Core/vars_statics.h"


Prac_a::Prac_a(QWidget *parent) : QWidget(parent), ui(new Ui::Prac_a) {

    ui->setupUi(this);

    QSettings settings(ivar::FILE_conf, QSettings::IniFormat);
    restoreGeometry(settings.value("dlgPract1").toByteArray());

    player = new QMediaPlayer(this);

    ui->pushButton_no->setIcon(QIcon(ivar::DS+"/images/no.png"));
    ui->pushButton_no->setText(tr("I did not know it"));
    ui->pushButton_ok->setIcon(QIcon(ivar::DS+"/images/yes.png"));
    ui->pushButton_ok->setText(tr("I Knew it"));
}


Prac_a::~Prac_a() {
    delete ui;
}


void Prac_a::load_data(std::map<QString, QString> &tmp_list_pair_words,
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
void Prac_a::cuestion_card() {

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


void Prac_a::answer_card() {

    trgt = list_words[count_pos];
    cuest = true;
    set_text_answer_card(trgt);
    count_pos++;

}

/* ------------------------------------------------ (3) */
void Prac_a::set_text_cuestion_card(QString trgt) {

    QFont font_trgt = ui->label_trgt->font();
    font_trgt.setPointSize(28);
    ui->label_trgt->setFont(font_trgt);

    QFont font_srce = ui->label_srce->font();
    font_srce.setPointSize(16);
    ui->label_srce->setFont(font_srce);

    ui->label_trgt->setText(trgt);
    ui->label_srce->setText("");

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

    ui->pushButton_answer->show();
}

void Prac_a::set_text_answer_card(QString trgt) {

    srce = list_pair_words[trgt];

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


void Prac_a::on_pushButton_no_clicked() { // no / next

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


void Prac_a::on_pushButton_answer_clicked() {

    answer_card();
}


void Prac_a::closeEvent( QCloseEvent* event ) {

    QSettings settings(ivar::FILE_conf, QSettings::IniFormat);
    settings.setValue("dlgPract1", saveGeometry());

    if(this->isVisible()){
        event->ignore();
        this->hide();
     }

    Practice dlg;
    dlg.go_back_results(count_quiz, list_easy, list_learning,
                                list_difficult, "Pract1");
    dlg.setModal(true);
    dlg.exec();
}


void Prac_a::on_label_trgt_clicked() {

    Audioplayer path;
    player->setMedia(QUrl::fromLocalFile(path.pathplay(trgt)));
    player->play();
}
