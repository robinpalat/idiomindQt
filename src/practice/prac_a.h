#ifndef PRAC_A_H
#define PRAC_A_H

#include <QWidget>
#include <QCloseEvent>
#include <QDebug>
#include <QFileInfo>
#include <QtSql/QtSql>
#include <iostream>
#include <fstream>
#include <QFileSystemWatcher>

#include "src/vars_statics.h"
#include "src/vars_session.h"
#include "src/database.h"


namespace Ui {
class Prac_a;
}

class Prac_a : public QWidget
{
    Q_OBJECT

public:
    explicit Prac_a(QWidget *parent = nullptr);
    ~Prac_a();

public:
    void load_data(QString tpc);
    QString get_tpc();
    void setLabelText_cuest(QString trgt);
    void setLabelText_answer(QString trgt);
    void cuestion_card();
    void answer_card();
    void save_data();
    QString tpc, trgt, srce, type;
    QString scr_total, scr_learnt, scr_easy, scr_ling, scr_hard;
    bool cuest;
    unsigned short int pos, total, round, ok_count = 0, no_count = 0;
    unsigned long int items;

    std::vector< QString > words;
    std::vector< QString > learning;
    std::vector< QString > learnt;
    std::vector< QString > easy;
    std::vector< QString > difficult;
    std::map<QString,QString> pair_words;
    std::map<QString,QString>::iterator it;

private slots:
    void on_pushButton_ok_clicked();
    void closeEvent(QCloseEvent * event);

    void on_pushButton_no_clicked();

    void on_pushButton_answer_clicked();

private:
    Ui::Prac_a *ui;
};

#endif // PRAC_A_H
