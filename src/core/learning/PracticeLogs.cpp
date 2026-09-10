#include "core/learning/PracticeLogs.h"

#include <QDir>
#include <QFile>
#include <QTextStream>

namespace idiomind {
namespace core {

namespace {

QString readAllOf(const QString &path)
{
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
        return {};
    return QString::fromUtf8(f.readAll());
}

bool writeAll(const QString &path, const QString &content)
{
    QFile f(path);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate))
        return false;
    f.write(content.toUtf8());
    return true;
}

QString logPath(const QString &confDir, int n)
{
    return confDir + QStringLiteral("/practice/log%1").arg(n);
}

} // namespace

bool PracticeLogs::load(const QString &confDir)
{
    m_raw1 = readAllOf(logPath(confDir, 1));
    m_raw2 = readAllOf(logPath(confDir, 2));
    m_raw3 = readAllOf(logPath(confDir, 3));
    rebuildSet(m_raw1, m_log1Set);
    rebuildSet(m_raw2, m_log2Set);
    rebuildSet(m_raw3, m_log3Set);
    return true;
}

bool PracticeLogs::save(const QString &confDir) const
{
    bool ok = true;
    ok = writeAll(logPath(confDir, 1), m_raw1) && ok;
    ok = writeAll(logPath(confDir, 2), m_raw2) && ok;
    ok = writeAll(logPath(confDir, 3), m_raw3) && ok;
    return ok;
}

bool PracticeLogs::clear(const QString &confDir)
{
    m_raw1.clear(); m_raw2.clear(); m_raw3.clear();
    m_log1Set.clear(); m_log2Set.clear(); m_log3Set.clear();
    return save(confDir);
}

QString PracticeLogs::rawLog(int n) const
{
    if (n == 1) return m_raw1;
    if (n == 2) return m_raw2;
    if (n == 3) return m_raw3;
    return {};
}

void PracticeLogs::setRawLog(int n, const QString &content)
{
    if (n == 1) { m_raw1 = content; rebuildSet(content, m_log1Set); }
    if (n == 2) { m_raw2 = content; rebuildSet(content, m_log2Set); }
    if (n == 3) { m_raw3 = content; rebuildSet(content, m_log3Set); }
}

bool PracticeLogs::inLog(int n, const QString &trgt) const
{
    if (n == 1) return m_log1Set.contains(trgt);
    if (n == 2) return m_log2Set.contains(trgt);
    if (n == 3) return m_log3Set.contains(trgt);
    return false;
}

void PracticeLogs::append(int n, const QString &trgt)
{
    if (trgt.isEmpty()) return;
    if (n == 1) {
        m_log1Set.insert(trgt);
        m_raw1 += (m_raw1.isEmpty() ? QString() : QStringLiteral("\n")) + trgt;
    } else if (n == 2) {
        m_log2Set.insert(trgt);
        m_raw2 += (m_raw2.isEmpty() ? QString() : QStringLiteral("\n")) + trgt;
    } else if (n == 3) {
        m_log3Set.insert(trgt);
        m_raw3 += (m_raw3.isEmpty() ? QString() : QStringLiteral("\n")) + trgt;
    }
}

void PracticeLogs::remove(const QString &trgt)
{
    auto remove = [](QString &raw, QSet<QString> &set, const QString &t) {
        if (!set.contains(t)) return;
        set.remove(t);
        QStringList lines = raw.split(QLatin1Char('\n'), Qt::SkipEmptyParts);
        lines.removeAll(t);
        raw = lines.join(QLatin1Char('\n'));
    };
    remove(m_raw1, m_log1Set, trgt);
    remove(m_raw2, m_log2Set, trgt);
    remove(m_raw3, m_log3Set, trgt);
}

QStringList PracticeLogs::entries(int n) const
{
    const QString &raw = (n == 1) ? m_raw1 : (n == 2) ? m_raw2 : m_raw3;
    return raw.split(QLatin1Char('\n'), Qt::SkipEmptyParts);
}

void PracticeLogs::rebuildSet(const QString &raw, QSet<QString> &set)
{
    set.clear();
    for (const QString &line : raw.split(QLatin1Char('\n'), Qt::SkipEmptyParts))
        set.insert(line);
}

} // namespace core
} // namespace idiomind
