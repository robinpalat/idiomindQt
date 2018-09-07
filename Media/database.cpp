#include "Media/database.h"

Database::Database() {

    QMutexLocker locker(&lock);

//    if(QFile::exists()) {

//    } else create();
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

QSqlDatabase Database::getConnection(QString dbpath) {

    QThread *currentThread = QThread::currentThread();
    if (!currentThread) {
        return QSqlDatabase();
    }

    const QString threadName = currentThread->objectName();

    if (connections.contains(currentThread)) {
        return connections.value(currentThread);
    } else {
        QSqlDatabase connection = QSqlDatabase::addDatabase("QSQLITE", threadName);
        connection.setDatabaseName(dbpath);
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
