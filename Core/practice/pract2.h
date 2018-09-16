#ifndef PRACT2_H
#define PRACT2_H

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
class Pract2;
}

class Pract2 : public QWidget
{
    Q_OBJECT

public:
    explicit Pract2(QWidget *parent = nullptr);
    ~Pract2();

public:
    //void load_data(QString tpc);
    QString get_tpc();
    void set_text_cuestion_card(QString trgt);
    void cuestion_card();
    void save_data();
    QString tpc, trgt, srce, type;
    unsigned long int size_easy, size_learnt, size_ling, size_hard;
    bool cuest;
    unsigned short int count_quiz, count_total, count_items, count_pos, count_round, count_ok = 0, count_no = 0;
    //unsigned long int count_items;

    std::vector< QString > list_easy;
    std::vector< QString > list_learnt;
    std::vector< QString > list_learning;
    std::vector< QString > list_difficult;
    std::vector< QString > list_srces;
    std::vector< QString > list_shufle;

    std::vector< QString > list_words;
    std::map<QString,QString> list_pair_words;

    void load_data(std::map<QString, QString> &tmp_list_pair_words,
                   std::vector< QString > &tmp_list_words);


private slots:
    void on_pushButton_ok_clicked();
    void closeEvent(QCloseEvent * event);

    void on_pushButton_no_clicked();

    void on_label_trgt_clicked();

private:
    Ui::Pract2 *ui;
    QMediaPlayer * player;
};
#endif // PRACT2_H
