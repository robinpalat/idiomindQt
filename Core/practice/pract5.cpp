#include "Core/practice/pract5.h"
#include "ui_pract5.h"
#include "dlg_practice.h"

#include <QTimer>
#include <iostream>
#include <sstream>

#include "Core/vars_statics.h"


Pract5::Pract5(QWidget *parent) : QWidget(parent), ui(new Ui::Pract5) {

    ui->setupUi(this);
    QSettings settings(ivar::FILE_conf, QSettings::IniFormat);
    restoreGeometry(settings.value("dlgPract5").toByteArray());
    player = new QMediaPlayer(this);
    ui->pushButton->setText(tr("Check"));
    //ui->label_trgt->setTextFormat(Qt::RichText);
}


Pract5::~Pract5() {
    delete ui;
}


void Pract5::load_data(std::map<QString, QString> &tmp_list_pair_words,
                       std::vector< QString > &tmp_list_words) {

    list_words = tmp_list_words;
    list_pair_words = tmp_list_pair_words;
    count_ok = 0;
    count_no = 0;
    count_items = list_words.size();
    count_quiz = count_items;
    count_pos = 0;
    count_round = 1;
    write_card();
}


void Pract5::write_card() {

    iwrite = true;
    ui->plainTextEdit->clear();
    ui->plainTextEdit->show();
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
        trgt = list_words[count_pos];
    }
    set_text_write_card(trgt);
}


void Pract5::check_card() {

    iwrite = false;
    QString trgt_in = ui->plainTextEdit->toPlainText();
    ui->plainTextEdit->hide();
    list_trgt_words.clear();
    list_trgt_words_in.clear();
    QString Word;
    for(short i = 0; i < trgt_in.length(); i++) {
        if(trgt_in[i] == ' ' && !Word.isEmpty()) {
           list_trgt_words_in.push_back(Word);
           if(trgt_in.length() == i) Word.clear();
        }
        else {
            Word += trgt_in[i];
        }
    }
    for(short i = 0; i < trgt.length(); i++) {
        if(trgt[i] == ' ' && !Word.isEmpty()) {
           list_trgt_words.push_back(Word);
           Word.clear();
        }
        else {
            Word += trgt[i];
        }
    }
    qDebug() << list_trgt_words_in;
    qDebug() << list_trgt_words;
    if ("porc" == "porc") {
        if (count_round == 1) {
            list_easy.push_back(trgt);
        }
        else if (count_round == 2) {
            list_learnt.push_back(trgt);
        }

        count_ok++;
    }
    else if ("porc" == "porc") {

        count_ok++;
    }
    else {

        count_no++;
    }
    set_text_check_card(trgt);
}


void Pract5::set_text_write_card(QString trgt) {

    QString useraud1=DM_tl+"/.share/audio/"+trgt.toLower()+"-1.mp3";
    QString useraud2=DM_tl+"/"+tpc+"/"+trgt.toLower()+".mp3";
    std::vector<QString> vect = hide_sent(trgt);
    QString _trgt;
    for ( auto it = vect.begin(); it != vect.end(); ++it  ) {
        _trgt += *it;
        _trgt += "      ";
   }
    ui->label_trgt->setText(_trgt);
    ui->label_trgt->setWordWrap(true);
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


void Pract5::set_text_check_card(QString trgt) {

    ui->label_trgt->setText(trgt);
    ui->label_trgt->setWordWrap(true);
}


void Pract5::on_pushButton_clicked() {

    if (iwrite == true) {
        count_pos++;
        ui->pushButton->setText(tr("Continue"));
        check_card();
    }
    else {
         ui->pushButton->setText(tr("Check"));
         write_card();
    }
}

QString Pract5::hide_word(const QString& in) {

    int n = 0; QString trgt;
    in.size();
    for(auto c : in) {
        if ( n == 0) {
           trgt += c;
        }
        else {
            trgt += " .";
        }
        n++;
    }
    return trgt;
}


std::vector<QString> Pract5::hide_sent(QString trgt) {
    std::vector<QString> Sentence;
    list_trgt_words.clear();
    QString Word;
    for(short i=0;i<trgt.length();i++){
        if(trgt[i] == ' ' && !Word.isEmpty()){
           Sentence.push_back(hide_word(Word));
           Word.clear();
        }
        else {
            Word += trgt[i];
        }
    }
    qDebug() << list_trgt_words;
    return Sentence;


}


void Pract5::closeEvent( QCloseEvent* event ) {

    QSettings settings(ivar::FILE_conf, QSettings::IniFormat);
    settings.setValue("dlgPract5", saveGeometry());
    if(this->isVisible()){
        event->ignore();
        this->hide();
     }
    Practice dlg;
    dlg.go_back_results(count_quiz, list_easy, list_learning,
                                list_difficult, "Pract5");
    dlg.setModal(true);
    dlg.exec();
}


void Pract5::on_label_trgt_clicked() {

    Audioplayer path;
    player->setSource(QUrl::fromLocalFile(path.pathplay(trgt)));
    player->play();
}
