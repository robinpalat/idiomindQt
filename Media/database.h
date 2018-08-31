#ifndef DATABASE_H
#define DATABASE_H

#include <QObject>
#include <QtSql/QtSql>
#include <QDebug>

#include "Core/vars_statics.h"
#include "Core/vars_session.h"

class Database : public QObject
{
    Q_OBJECT
public:
    explicit Database(QObject *parent = nullptr);

signals:

public slots:

public:

    QSqlDatabase mdb;

    bool Opendb(QString db) {

        mdb=QSqlDatabase::addDatabase("QSQLITE");
        mdb.setDatabaseName(db);
        if (!mdb.open()) {
            qDebug()<<("Failed to open the database");
            return false;
        }
        else {
            return true;
        }
    }

    void Closedb() {

        QSqlQuery qry;
        qry.finish();
        mdb.close();
        mdb = QSqlDatabase();
        mdb.removeDatabase(QSqlDatabase::defaultConnection);
        if(mdb.open()) {
            qDebug() << "Failed to close the databse";
        }
    }

};

#endif // DATABASE_H
