// F2.1 - Review: fila `reviews` de un topic (date1..date10).
// Autoridad: docs/04-databases.md, docs/06-learning-system.md, ifs/cmns.sh
// calculate_review, mngr.sh mark_as_learned.
//
// Fechas sin columna vacía. El original usa dos formatos distintos:
//   - SQLite (tpc_db): texto "%m/%d/%Y"   (date +%m/%d/%Y)
//   - .idmnd:            texto "%Y-%m-%d" (dtec/dteu/dtei)
// Esta clase NO convierte entre formatos en silencio: el llamador elige
// explícitamente setSqliteDate()/setIsoDate().
#pragma once

#include <QDate>
#include <QString>

class Review {
public:
    static constexpr int MaxDates = 10;  // date1..date10

    Review() = default;

    // Número de columnas 1..10 no vacías (equivalente al
    // `grep -c '[^[:space:]]'` del original sobre la fila TEXT).
    int countFilled() const;

    // Columna 1..10 (1-based, date1..date10). Inválida si está vacía.
    QDate dateAt(int column) const;
    void setDate(int column, const QDate &d);

    // Última fecha mencionada, con la semántica del original:
    // date[countFilled] (calculate_review). Inválida si no hay fechas.
    QDate lastDate() const;

    // -- formatos explícitos, sin magia --------------------------------------
    QString sqliteDate(int column) const;          // "%m/%d/%Y" o vacío
    void setSqliteDate(int column, const QString &s);
    QString isoDate(int column) const;             // "%Y-%m-%d" o vacío
    void setIsoDate(int column, const QString &s);

    static QString toSqliteDate(const QDate &d);   // "%m/%d/%Y"
    static QDate fromSqliteDate(const QString &s, bool *ok = nullptr);
    static QString toIsoDate(const QDate &d);      // "%Y-%m-%d"
    static QDate fromIsoDate(const QString &s, bool *ok = nullptr);

    bool operator==(const Review &o) const;

private:
    QDate m_dates[MaxDates];
};