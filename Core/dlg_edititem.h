#ifndef DLG_EDITITEM_H
#define DLG_EDITITEM_H

#include <QDialog>
#include <QtSql/QtSql>

#include <iostream>
#include "vars_statics.h"
#include "vars_session.h"
#include "Media/database.h"

namespace Ui {
class Dlg_editItem;
}

class Dlg_editItem : public QDialog
{
    Q_OBJECT

public:
    explicit Dlg_editItem(QWidget *parent = nullptr);
    ~Dlg_editItem();

private:
    Ui::Dlg_editItem *ui;


public:

    Global mGlobal;
    QString tpc = mGlobal.get_textline(ivar::FILE_mn);

public:
    unsigned long int pos = 0, items =0;
    int temp;
    int numberOfRows = 0, checkpos;
    bool loaded, mod = false;
    std::vector< QString > arr;
    QString trgt, srce, list, grmr,
    wrds, type, note, defn, exmp, mark;
    void load_data(QString trgt, QString list);
    void fill_data(QString trgt);
    void save_data();

private slots:
    void on_pushButton_editItem_close_clicked();
    void on_pushButton_editItem_prev_clicked();
    void on_pushButton_editItem_next_clicked();
    void closeEvent(QCloseEvent * event);

private:

};

#endif // DLG_EDITITEM_H
