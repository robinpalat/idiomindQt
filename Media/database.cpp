#include "Media/database.h"

Database::Database() {

    QMutexLocker locker(&lock);

}


Database::~Database() {

    closeConnections();
}


Database& Database::instance() {

    static QMutex mutex;
    QMutexLocker locker(&mutex);
    static Database *databaseInstance = new Database();
    return *databaseInstance;
}


QSqlDatabase Database::getConnection(QString tpc) {

    QThread *currentThread = QThread::currentThread();
    if (!currentThread) {
        return QSqlDatabase();
    }
//    if (in == 1){
//        QSqlDatabase connection = QSqlDatabase::addDatabase("QSQLITE", threadName);
//        connection.setDatabaseName(DM_tl+"/"+tpc+"/.conf/tpc");
//        if(!connection.open()) {
//            qWarning() << QString("Cannot connect to database")
//                              .arg(connection.databaseName(), threadName);
//        }
//        connections.insert(currentThread, connection);
//        return connection;
//    }
    const QString threadName = currentThread->objectName();
    qDebug() << threadName;
    if (connections.contains(currentThread)) {
        return connections.value(currentThread);

    } else {
        QSqlDatabase connection = QSqlDatabase::addDatabase("QSQLITE", threadName);
        connection.setDatabaseName(DM_tl+"/"+tpc+"/.conf/tpc");
        if(!connection.open()) {
            qWarning() << QString("Cannot connect to database")
                              .arg(connection.databaseName(), threadName);
        }
        connections.insert(currentThread, connection);
        return connection;
    }
}



void Database::closeConnections() {

    foreach(QSqlDatabase connection, connections.values()) {
        connection.close();
    }
    connections.clear();
}


void Database::closeConnection() {

    QThread *currentThread = QThread::currentThread();
    if (!connections.contains(currentThread)) return;
    QSqlDatabase connection = connections.take(currentThread);
    connection.close();
}
