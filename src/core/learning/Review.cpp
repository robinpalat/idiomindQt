#include "core/learning/Review.h"

QString Review::toSqliteDate(const QDate &d)
{
    return d.isValid() ? d.toString(QStringLiteral("MM/dd/yyyy")) : QString();
}

QDate Review::fromSqliteDate(const QString &s, bool *ok)
{
    const QDate d = QDate::fromString(s, QStringLiteral("MM/dd/yyyy"));
    if (ok)
        *ok = d.isValid();
    return d;
}

QString Review::toIsoDate(const QDate &d)
{
    return d.isValid() ? d.toString(Qt::ISODate) : QString();
}

QDate Review::fromIsoDate(const QString &s, bool *ok)
{
    const QDate d = QDate::fromString(s, Qt::ISODate);
    if (ok)
        *ok = d.isValid();
    return d;
}

int Review::countFilled() const
{
    int n = 0;
    for (int i = 0; i < MaxDates; ++i)
        if (m_dates[i].isValid())
            ++n;
    return n;
}

QDate Review::dateAt(int column) const
{
    if (column < 1 || column > MaxDates)
        return {};
    return m_dates[column - 1];
}

void Review::setDate(int column, const QDate &d)
{
    if (column < 1 || column > MaxDates)
        return;
    m_dates[column - 1] = d;
}

QDate Review::lastDate() const
{
    const int n = countFilled();
    return n > 0 ? m_dates[n - 1] : QDate();
}

QString Review::sqliteDate(int column) const
{
    return toSqliteDate(dateAt(column));
}

void Review::setSqliteDate(int column, const QString &s)
{
    bool ok = false;
    const QDate d = fromSqliteDate(s, &ok);
    if (ok)
        setDate(column, d);
}

QString Review::isoDate(int column) const
{
    return toIsoDate(dateAt(column));
}

void Review::setIsoDate(int column, const QString &s)
{
    bool ok = false;
    const QDate d = fromIsoDate(s, &ok);
    if (ok)
        setDate(column, d);
}

bool Review::operator==(const Review &o) const
{
    for (int i = 0; i < MaxDates; ++i)
        if (m_dates[i] != o.m_dates[i])
            return false;
    return true;
}