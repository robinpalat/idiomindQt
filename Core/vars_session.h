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
#include <map>
#include <algorithm>
#include <string>


extern QString tpc;
extern QString FILE_mn;
extern QString config_db;

extern QString tlng;
extern QString slng;

extern QString DM_tl;
extern QString DM_tlt;
extern QString DC_tlt;
extern QString DC_tls;

extern QString FILE_shrdb;
extern QString FILE_tlngdb;
extern QString FILE_tpcdb;

extern QString Source_LANG;
extern QString Home;

class Global
{
public:
    Global();
    void get_vars();
    QString get_textline(QString);

};

#endif // GLAOBAL_H
