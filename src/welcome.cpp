#include "welcome.h"
#include "ui_welcome.h"

#include "vars_statics.h"
#include "vars_session.h"
#include <string>
#include <iostream>
#include <map>

using namespace std;


Welcome::Welcome(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Welcome)
{
    ui->setupUi(this);
}

Welcome::~Welcome()
{
    delete ui;
}

bool Welcome::create_user(){

    if (!QDir(ivar::DM).exists()) QDir().mkdir(ivar::DM);
    if (!QDir(ivar::DM_t).exists()) QDir().mkdir(ivar::DM_t);

    // share db in "$HOME/.idiomind/topics/$LANGUA/data/config";
    // related to configuration of topics in $LANGUA
    if (!QFile(FILE_shrdb).exists()) {

        QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
        db.setDatabaseName(FILE_shrdb);
        db.open();
        QSqlQuery query;
        query.exec("create table person "
                  "(id integer primary key, "
                  "firstname varchar(20), "
                  "lastname varchar(30), "
                  "age integer)");
    }

    // share db in "$HOME/.idiomind/topics/$LANGUA/data/$LANGUA.db";
    // related to translation of words in $LANGUA
    if (!QFile(FILE_tlngdb).exists()) {

        QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
        db.setDatabaseName(FILE_tlngdb);
        db.open();
        QSqlQuery query;
        query.exec("create table person "
                  "(id integer primary key, "
                  "firstname varchar(20), "
                  "lastname varchar(30), "
                  "age integer)");
    }


    if (!QDir(ivar::DT).exists()) QDir().mkdir(ivar::DT);
    if (!QDir(ivar::DM).exists()) QDir().mkdir(ivar::DM);
    if (!QDir(ivar::DT).exists()) QDir().mkdir(ivar::DT);
    if (!QDir(ivar::DT).exists()) QDir().mkdir(ivar::DT);

    std::map <string, string> tlangs;
    tlangs["English"] = "en";
    tlangs["Spanish"] = "es";
    tlangs["Italian"] = "it";
    tlangs["Portuguese"] = "pt";
    tlangs["German"] = "de";
    tlangs["Japanese"] = "ja";
    tlangs["French"] = "fr";
    tlangs["Vietnamese"] = "vi";
    tlangs["Chinese"] = "en";
    tlangs["Russian"] = "ru";

    cout << tlangs["Chinese"] << endl;
    cout << tlangs["Italian"] << endl;
}



void Welcome::on_pushButton_2_clicked()
{
    create_user();
    this->close();
    accept();
}

void Welcome::on_pushButton_clicked()
{
    this->close();
    reject();
}
