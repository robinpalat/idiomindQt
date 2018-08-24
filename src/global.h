#ifndef GLOBAL_H
#define GLOBAL_H
#include <qstring.h>

#include <QObject>
#include <QWidget>
#include <QMessageBox>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QtSql/QtSql>

#include <iostream>

extern QString tpc;
extern QString Username;
extern QString Home;
extern QString DT;
extern QString DS;
extern QString DS_a;
extern QString FILE_mn;

extern QString tlng;
extern QString slng;

extern QString DM;
extern QString DM_t;
extern QString DM_tl;
extern QString DM_tlt;
extern QString DC_tlt;

extern QString DC_tls;

extern QString DC;
extern QString DC_a;
extern QString DC_d;

extern QString FILE_shrdb;
extern QString FILE_tlngdb;
extern QString FILE_tpcdb;


class Global
{
public:
    Global();
    void get_vars();
    QString get_textline(QString);



};

#endif // GLAOBAL_H
