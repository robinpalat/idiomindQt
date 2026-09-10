#include "app/Application.h"

#include <QDebug>

namespace idiomind {

Application::Application(QObject* parent)
    : QObject(parent)
    , m_paths()
    , m_settings()
    , m_topicProxy(new ui::TopicProxy(m_paths, this))
{
    qInfo().noquote()
        << "Idiomind Qt" << appVersion()
        << "| dataRoot:" << m_paths.dataRoot()
        << "| configRoot:" << m_paths.configRoot()
        << "| tmpDir:" << m_paths.tmpDir();

    // F6-B.8: establecer idioma por defecto y cargar topic persistido
    m_topicProxy->setActiveLanguage(QStringLiteral("English"));
    m_topicProxy->loadActiveTopic();
}

QString Application::appName()
{
    return QStringLiteral("Idiomind");
}

QString Application::appVersion()
{
    return QStringLiteral("0.1.0");
}

QString Application::name() const
{
    return appName();
}

QString Application::version() const
{
    return appVersion();
}

QString Application::pathsSummary() const
{
    return QStringLiteral(
               "dataRoot  %1\n"
               "configRoot %2\n"
               "topics    %3\n"
               "backup    %4\n"
               "tmpDir    %5")
        .arg(m_paths.dataRoot(), m_paths.configRoot(), m_paths.topicsDir(),
             m_paths.backupDir(), m_paths.tmpDir());
}

} // namespace idiomind