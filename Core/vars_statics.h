#ifndef SESSION_H
#define SESSION_H

#include <QObject>
#include <QWidget>
#include <QMessageBox>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QtSql/QtSql>


#include <iostream>

#include "welcome.h"

namespace ivar {
    extern QString Username;

    extern QString Home;
    extern QString DT;
    extern QString DS;
    extern QString DS_a;
    extern QString DM;
    extern QString DM_t;
    extern QString DC;
    extern QString DC_a;
    extern QString DC_d;

     extern QString FILE_mn;
     extern QString FILE_conf;

    extern QString slangs[47][47];
}


class Session : public QObject
{
    Q_OBJECT


public:
    explicit Session(QObject *parent = nullptr);


public:


signals:


public slots:


private:
   // Welcome * mWelcome;

};

#endif // SESSION_H


