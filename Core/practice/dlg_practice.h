#ifndef DLG_PRACTICE_H
#define DLG_PRACTICE_H

#include <QDialog>
#include <QTableWidget>

#include "Core/practice/pract1.h"
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
    void load_data();
    void save_data();
    void startt();

    void score_info(unsigned long int total,
                    unsigned long int easy,
                    unsigned long int ling,
                    unsigned long int hard);

    unsigned long int int_total, int_easy, int_ling, int_hard;
    unsigned short int pos, total, round, ok_count = 0, no_count = 0;
    unsigned long int items;

    std::vector< QString > words;
    std::map<QString,QString> pair_words;


    std::map<QString,QString> img_pair_practs;

    QString practs[5] = {tr("Flashcards"),tr("Multiple-choice"),
                        tr("Recognize Pronunciation"),
                        tr("Images"),tr("Listen and Writing Sentences")};



private slots:
    void on_pushButton_clicked();
    void closeEvent(QCloseEvent * event);

    void on_tableWidget_itemDoubleClicked(QTableWidgetItem *item);


private:
    Ui::Practice *ui;
    Prac_a * mPrac_a;

};

#endif // DLG_PRACTICE_H
