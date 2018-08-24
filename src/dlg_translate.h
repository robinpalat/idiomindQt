#ifndef DLG_TRANSLATE_H
#define DLG_TRANSLATE_H

#include <QDialog>
#include <QSql>
#include <QTableWidgetItem>
#include <QGroupBox>
#include <QFormLayout>
#include <QLineEdit>
#include <QTextEdit>

#include "session.h"
#include "global.h"

namespace Ui {
class Translate;
}

class Translate : public QDialog
{
    Q_OBJECT

public:
    explicit Translate(QWidget *parent = nullptr);
    ~Translate();

public:
    void load_data();
    void save_data();
    QString get_tpc();
    QString tpc;
    std::vector< QString > trans_load_items;
    std::vector< QString > trans_check_items;


    void connCloseb()
    {
        QSqlDatabase mdb;
        QSqlQuery qry;
        qry.finish();
        mdb.close();
        mdb = QSqlDatabase();
        mdb.removeDatabase(QSqlDatabase::defaultConnection);
        if(mdb.open()) std::cout << "Failed to close the databse... (edit)" << std::endl;
        else std::cout << "Database closed... (edit)" << std::endl;
    }

    bool connOpenb()
    {
        QSqlDatabase mdb;
        mdb=QSqlDatabase::addDatabase("QSQLITE");

        // mdb.setDatabaseName("/sdcard/Ankidroid/tpc");
        //mdb.setDatabaseName(ivar::DC_tlt+"/tpcdb");
        mdb.setDatabaseName(ivar::DM_tl+"/"+tpc+"/.conf/tpcdb");

        if (!mdb.open())
        {
            qDebug()<<("Failed to open the database (edit)");
            return false;
        }
        else
        {
            qDebug()<<("Connected... (edit)");
            return true;
        }
    }

private slots:
    void on_pushButton_close_clicked();

    void on_pushButton_save_trans_clicked();

private:
    Ui::Translate *ui;
    QVBoxLayout *vbox;
};

#endif // DLG_TRANSLATE_H
