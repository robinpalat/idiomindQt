#ifndef Pract5_H
#define Pract5_H

#include <QWidget>
#include <QCloseEvent>
#include <QDebug>
#include <QFileInfo>
#include <QtSql/QtSql>
#include <iostream>
#include <fstream>
#include <QFileSystemWatcher>
#include <QMediaPlayer>

#include "Classes/adds_clicklabel.h"
#include "Core/vars_statics.h"
#include "Core/vars_session.h"
#include "Media/database.h"
#include "Media/audioplayer.h"


namespace Ui {
class Pract5;
}

class Pract5 : public QWidget
{
    Q_OBJECT

public:
    explicit Pract5(QWidget *parent = nullptr);
    ~Pract5();

public:
    //void load_data(QString tpc);
    QString get_tpc();
    void set_text_write_card(QString trgt);
    void set_text_check_card(QString trgt);
    void write_card();
    void check_card();
    void save_data();
    QString hide_word(const QString& infix);
    std::vector<QString> hide_sent(QString infix);
    QString tpc, trgt, srce, type;
    unsigned long int size_easy, size_learnt, size_ling, size_hard;
    bool iwrite;
    unsigned short int count_quiz, count_items,
    count_pos, count_round, count_ok = 0, count_no = 0;
    //unsigned long int count_items;
    std::vector< QString > list_easy;
    std::vector< QString > list_learnt;
    std::vector< QString > list_learning;
    std::vector< QString > list_difficult;
    std::vector< QString > list_words;
    std::vector< QString > list_trgt_words;
    std::vector< QString > list_trgt_words_in;
    std::map<QString,QString> list_pair_words;
    void load_data(std::map<QString, QString> &tmp_list_pair_words,
                   std::vector< QString > &tmp_list_words);


private slots:
    void on_pushButton_clicked();
    void on_label_trgt_clicked();
    void closeEvent(QCloseEvent * event);

private:
    Ui::Pract5 *ui;
    QMediaPlayer * player;
};

#endif // Pract5_H
