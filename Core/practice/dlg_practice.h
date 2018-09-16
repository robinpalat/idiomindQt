#ifndef DLG_PRACTICE_H
#define DLG_PRACTICE_H

#include <QDialog>
#include <QTableWidget>

#include "Core/practice/pract1.h"
#include "Core/practice/pract2.h"
#include "Core/practice/pract3.h"
#include "Core/practice/pract4.h"
#include "Core/practice/pract5.h"
#include "Media/database.h"

namespace Ui {
class Practice;
}

class Practice : public QDialog
{
    Q_OBJECT

public:
    explicit Practice(QWidget *parent = nullptr);
    ~Practice();

public:
    QString tpc, active_pract, nicon, nicon_mod;
    QString get_tpc();
    Database conn;
    void show_icons_stats();
    void save_data();
    void get_icons_stats();
    void starting_a_pract(QString pract);
    void practice_is_21_0(QString active_pract);

    void go_back_results(unsigned short int count_quiz,
                        std::vector< QString > &list_easy,
                        std::vector< QString > &list_learning,
                        std::vector< QString > &list_difficult,
                        QString active_pract);

    void calc_score_data(QString active_pract);

    unsigned long int size_easy, size_lrnt, size_ling, size_hard;
    unsigned short int count_pos,
    count_round, count_ok = 0, count_no = 0;
    unsigned long int items;
    unsigned count_session, count_quiz, count_learnt;


    std::vector< QString > list_total;
    std::vector< QString > list_easy;
    std::vector< QString > list_learning;
    std::vector< QString > list_difficult;
    std::vector< QString > list_words;

    std::map<QString,QString> list_pair_words;
    std::map<QString,QString> list_pair_icon_practs;

    QString practs[5] = {tr("Flashcards"),tr("Multiple-choice"),
                        tr("Recognize Pronunciation"),
                        tr("Images"),tr("Listen and Writing Sentences")};



private slots:
    void on_pushButton_restart_clicked();
    void closeEvent(QCloseEvent * event);

    void on_tableWidget_itemDoubleClicked(QTableWidgetItem *item);


private:
    Ui::Practice *ui;
    Prac_a * mPrac_a;
    Pract2 * mPract2;
    Pract3 * mPract3;
    Pract4 * mPract4;
    Pract5 * mPract5;

};

#endif // DLG_PRACTICE_H
