#ifndef DLG_TOPICS_H
#define DLG_TOPICS_H

#include <QDialog>

#include "global.h"

namespace Ui {
class Topics;
}

class Topics : public QDialog
{
    Q_OBJECT

public:
    explicit Topics(QWidget *parent = nullptr);
    ~Topics();

private slots:
    void on_pushButton_clicked();
    void closeEvent(QCloseEvent * event);



    void on_tableWidget_topics_cellDoubleClicked();

    void on_pushButton_2_clicked();

private:
    Ui::Topics *ui;
    Global *mGlobal;


public:
    void load_index();

};

#endif // DLG_TOPICS_H
