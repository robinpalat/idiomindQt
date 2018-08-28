#ifndef DLG_VWR_H
#define DLG_VWR_H

#include <QDialog>
#include <sys/stat.h>

#include <QtSql/QtSql>
#include <QDebug>
#include <QSql>
#include <QMessageBox>
#include <QMediaPlayer>

#include <iostream>
#include <fstream>
#include <string>

#include <time.h>

#include "dlg_imageview.h"
#include "dlg_edititem.h"
#include "adds_clicklabel.h"
#include "vars_session.h"

using namespace std;



namespace Ui {
class Vwr;

}

class Vwr : public QDialog
{
    Q_OBJECT

public:
    explicit Vwr(QWidget *parent = nullptr);
    ~Vwr();


public:

    QSqlDatabase mydb;
    Global mGlobal;
    QString tpc = mGlobal.get_textline(ivar::FILE_mn);

    void connClose()
    {
        QSqlQuery qry;
        qry.finish();
        mydb.close();
        mydb = QSqlDatabase();
        mydb.removeDatabase(QSqlDatabase::defaultConnection);
        if(mydb.open()) cout << "Failed to close the databse..." << endl;
        else cout << "Database closed..." << endl;
    }

    bool connOpen()
    {
        mydb=QSqlDatabase::addDatabase("QSQLITE");

        // mydb.setDatabaseName("/sdcard/Ankidroid/tpc");
       // mydb.setDatabaseName(ivar::DC_tlt+"/tpcdb");
        mydb.setDatabaseName(DM_tl+"/"+tpc+"/.conf/tpcdb");

        if (!mydb.open())
        {
            qDebug()<<("Failed to open the database");
            return false;
        }
        else
        {
            qDebug()<<("Connected...");
            return true;
        }
    }


public:
    unsigned long int pos = 0, items =0;
    int temp;
    int numberOfRows = 0, checkpos;
    bool loaded;
    std::vector< QString > arr;
    QString trgt, srce, list, grmr, cdid,
    wrds, type, note, defn, exmp;
    void load_array(QString, QString);
    void sleepcp(int milliseconds);


public:
    void setLabelText(QString);
    void setPos(QString);
    string int_array_to_string(string int_array[], int size_of_array);
    void setConnect_lABEL_IMAGE();


public:
    QSqlQueryModel * modal=new QSqlQueryModel();


private slots:
    void on_pushButton_next_clicked();
    void on_pushButton_prev_clicked();
    void on_label_image_clicked();
    void on_label_trgt_clicked();
    void on_label_srce_clicked();
    void on_label_note_clicked();
    //void on_pushButton_edit_clicked();
    //void closeEvent( QCloseEvent* event );

    //void on_pushButton_2_clicked();


private:
    Ui::Vwr *ui;
    Dlg_ImageView * mDlg_ImageView;
    Dlg_editItem * mDlg_editItem;
    QMediaPlayer * player;

};

#endif // VWRDIALOG_H
