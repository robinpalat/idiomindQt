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

#include "../vars_statics.h"
#include "../vars_session.h"


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
    bool cuest;
    unsigned long int pos, items, round;

    std::vector< QString > words;
    std::vector< QString > learning;
    std::vector< QString > learnt;
    std::vector< QString > easy;
    std::vector< QString > difficult;
    std::map<QString,QString> pair_words;
    std::map<QString,QString>::iterator it;

private slots:
    void on_pushButton_nex_clicked();
    void closeEvent(QCloseEvent * event);

    void on_pushButton_exit_clicked();

private:
    Ui::Prac_a *ui;
};

#endif // PRAC_A_H
