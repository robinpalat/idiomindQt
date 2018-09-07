#include "prac_a.h"
#include "build/ui/ui_prac_a.h"
#include "dlg_practice.h"

Prac_a::Prac_a(QWidget *parent) : QWidget(parent), ui(new Ui::Prac_a) {

    ui->setupUi(this);

    player = new QMediaPlayer(this);

    ui->pushButton_no->setIcon(QIcon(ivar::DS+"/images/no.png"));
    ui->pushButton_no->setText(tr("I did not know it"));
    ui->pushButton_ok->setIcon(QIcon(ivar::DS+"/images/yes.png"));
    ui->pushButton_ok->setText(tr("I Knew it"));
}


Prac_a::~Prac_a() {
    delete ui;
}

void Prac_a::load_data(std::map<QString, QString> &tmp_pair_words,
                       std::vector< QString > &tmp_words) {

    words = tmp_words;
    pair_words = tmp_pair_words;
    total = words.size();
    qDebug() << total;
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

void Prac_a::on_pushButton_answer_clicked() {

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


void Prac_a::closeEvent( QCloseEvent* event ) {

    if(this->isVisible()){
        event->ignore();
        this->hide();
     }
    int_easy =  easy.size();
    int_ling = learning.size();
    int_learnt = learnt.size();
    int_hard = difficult.size();

    Practice * mPractice;
    mPractice = new Practice(this);
    if(items < 1){
        mPractice->score_info(total, int_easy, int_ling, int_hard);
    }
    else{
        mPractice->score_info(total, int_easy, int_ling, int_hard);
    }
    mPractice->show();
}

void Prac_a::on_label_trgt_clicked() {

    Audioplayer path;
    player->setMedia(QUrl::fromLocalFile(path.pathplay(trgt)));
    player->play();

}

