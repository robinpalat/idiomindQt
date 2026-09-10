// F2.2 - SqliteDatabase: envoltura mínima de QSqlDatabase (driver QSQLITE).
// Autoridad: docs/04-databases.md, docs/09-qt-architecture.md §4 (storage/dbs).
//
// Reglas heredadas del original (mkdb.sh / c.conf):
//  - SIN WAL (compat: el original usa journal por defecto; nunca activar WAL).
//  - el esquema lo define mkdb.sh ("no schema migration"); esta clase solo
//    ejecuta SQL.
//  - Las peticiones preparadas se ejecutan con `exec`/`prepare` de Qt; el
//    escaping de comillas lo hace QSqlQuery (el Bash usaba sed s/'/''/g).
//
// Lectura de un valor escalar: se usa QSqlQuery válido (sin vember conclusiones
// sobre filas inexistentes; el original toleraba celdas vacías por defecto).
#pragma once

#include <QSqlQuery>
#include <QString>

class QSqlDatabase;

namespace idiomind {
namespace storage {

class SqliteDatabase
{
public:
    explicit SqliteDatabase(QString path);
    ~SqliteDatabase();

    SqliteDatabase(const SqliteDatabase &) = delete;
    SqliteDatabase &operator=(const SqliteDatabase &) = delete;

    bool open();
    void close();
    bool isOpen() const { return m_isOpen; }

    const QString &path() const { return m_path; }

    // Ejecuta una sentencia sin resultado (CREATE/INSERT/UPDATE/DELETE/PRAGMA).
    bool execute(const QString &sql);

    // Ejecuta una consulta SELECT (o cualquier otra) y devuelve el QSqlQuery
    // posicionado en la primera fila (o `isActive()` válido sin filas).
    QSqlQuery query(const QString &sql);

    // Lee un campo escalar (SELECT <col> FROM <table>) como texto (SQLite
    // TEXT almacena casi todo en este esquema; los números se leen de igual
    // forma que sqlite3 CLI). Devuelve cadena vacía si no hay filas/col NULL.
    QString value(const QString &sql);

private:
    QString m_path;
    QString m_connectionName;
    bool m_isOpen = false;
};

} // namespace storage
} // namespace idiomind