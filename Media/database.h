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
    static Database& instance();
    QSqlDatabase getConnection(QString dbpath);
    ~Database();
     Database();
    void closeConnections();
    void closeConnection();
    QSqlDatabase mdb;
    bool Opendb(QString dbpath) {
        mdb = QSqlDatabase::addDatabase("QSQLITE");
        mdb.setDatabaseName(dbpath);
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

private:
    QMutex lock;
    QHash<QThread*, QSqlDatabase> connections;

};

#endif // DATABASE_H
