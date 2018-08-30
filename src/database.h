#ifndef DATABASE_H
#define DATABASE_H

#include <QObject>
#include <QtSql/QtSql>
#include <QDebug>

#include "vars_statics.h"
#include "vars_session.h"

class Database : public QObject
{
    Q_OBJECT
public:
    explicit Database(QObject *parent = nullptr);

signals:

public slots:

public:

    QSqlDatabase mdb;

    bool Opendb(QString db)
    {
        mdb=QSqlDatabase::addDatabase("QSQLITE");
        mdb.setDatabaseName(db);
        //mdb.setDatabaseName(DM_tl+"/"+tpc+"/.conf/tpcdb");

        if (!mdb.open()) {

            qDebug()<<("Failed to open the database (edit)");
            return false;
        }
        else {
            qDebug()<<("Connected");
            return true;
        }
    }

    void Closedb()
    {
        QSqlQuery qry;
        qry.finish();
        mdb.close();
        mdb = QSqlDatabase();
        mdb.removeDatabase(QSqlDatabase::defaultConnection);

        if(mdb.open()) {

            qDebug() << "Failed to close the databse... (edit)";
        }
        else {
            qDebug() << "Database closed";
        }
    }

};

#endif // DATABASE_H
