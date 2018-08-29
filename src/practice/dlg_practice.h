#ifndef DLG_PRACTICE_H
#define DLG_PRACTICE_H

#include <QDialog>
#include <QTableWidget>

#include "src/practice/prac_a.h"

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
    void load_data();
    QString tpc;
    QString get_tpc();


private slots:
    void on_pushButton_clicked();
    void closeEvent(QCloseEvent * event);

    void on_tableWidget_itemDoubleClicked(QTableWidgetItem *item);

private:
    Ui::Practice *ui;
    Prac_a * mPrac_a;
};

#endif // DLG_PRACTICE_H
