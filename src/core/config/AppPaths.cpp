#include "core/config/AppPaths.h"

#include <QDir>
#include <QProcessEnvironment>

namespace idiomind {
namespace core {

AppPaths::AppPaths(QString homeDir, QString tmpRoot)
    : m_homeDir(std::move(homeDir))
    , m_tmpRoot(std::move(tmpRoot))
    , m_userName(userFromEnvironment())
{
}

QString AppPaths::homeDir() const
{
    return m_homeDir;
}

QString AppPaths::dataRoot() const
{
    return join(m_homeDir, QStringLiteral(".idiomind"));
}

QString AppPaths::configRoot() const
{
    return join(m_homeDir, QStringLiteral(".config/idiomind"));
}

QString AppPaths::topicsDir() const
{
    return join(dataRoot(), QStringLiteral("topics"));
}

QString AppPaths::backupDir() const
{
    return join(dataRoot(), QStringLiteral("backup"));
}

QString AppPaths::topicsDirForLanguage(const QString& tlng) const
{
    return join(topicsDir(), tlng);
}

QString AppPaths::sharedDirForLanguage(const QString& tlng) const
{
    return join(topicsDirForLanguage(tlng), QStringLiteral(".share"));
}

QString AppPaths::topicDataDir(const QString& tlng, const QString& topic) const
{
    return join(topicsDirForLanguage(tlng), topic);
}

QString AppPaths::topicConfDir(const QString& tlng, const QString& topic) const
{
    // F2.2: DC_tlt = $DM_tl/<topic>/.conf (c.conf línea 25). El diseño F0 lo
    // ponía en configRoot sin idioma: corregido al comportamiento real.
    return join(topicDataDir(tlng, topic), QStringLiteral(".conf"));
}

QString AppPaths::configDb() const
{
    return join(configRoot(), QStringLiteral("config"));
}

QString AppPaths::sharedDataDir(const QString& tlng) const
{
    return join(sharedDirForLanguage(tlng), QStringLiteral("data"));
}

QString AppPaths::sharedDb(const QString& tlng) const
{
    return join(sharedDataDir(tlng), QStringLiteral("config"));
}

QString AppPaths::tlngDb(const QString& tlng) const
{
    return join(sharedDataDir(tlng), tlng + QStringLiteral(".db"));
}

QString AppPaths::topicDb(const QString& tlng, const QString& topic) const
{
    return join(topicConfDir(tlng, topic), QStringLiteral("tpc"));
}

QString AppPaths::tmpDir() const
{
    return join(m_tmpRoot, QStringLiteral(".idiomind-") + m_userName);
}

QString AppPaths::defaultHomeDir()
{
    return QDir::homePath();
}

QString AppPaths::defaultTempRoot()
{
    return QDir::tempPath();
}

QString AppPaths::userFromEnvironment()
{
    const QString user = qEnvironmentVariable("USER");
    return user.isEmpty() ? QStringLiteral("user") : user;
}

QString AppPaths::join(const QString& base, const QString& child)
{
    return QDir::cleanPath(base + QLatin1Char('/') + child);
}

} // namespace core
} // namespace idiomind