#include "core/topic/TopicList.h"

#include <QDir>

namespace idiomind {
namespace core {

bool TopicList::add(const Topic &topic)
{
    if (topic.name().isEmpty() || findByName(topic.name()) != nullptr)
        return false;
    m_topics.append(topic);
    return true;
}

bool TopicList::remove(const QString &name)
{
    for (auto it = m_topics.begin(); it != m_topics.end(); ++it) {
        if (it->name() == name) {
            m_topics.erase(it);
            return true;
        }
    }
    return false;
}

Topic *TopicList::findByName(const QString &name)
{
    for (Topic &t : m_topics) {
        if (t.name() == name)
            return &t;
    }
    return nullptr;
}

const Topic *TopicList::findByName(const QString &name) const
{
    return const_cast<TopicList *>(this)->findByName(name);
}

Topic *TopicList::findByPath(const QString &topicPath, const QString &topicsRoot)
{
    return const_cast<Topic *>(
        const_cast<const TopicList *>(this)->findByPath(topicPath, topicsRoot));
}

const Topic *TopicList::findByPath(const QString &topicPath,
                                   const QString &topicsRoot) const
{
    if (topicPath.isEmpty())
        return nullptr;

    const QString cleaned = QDir::cleanPath(topicPath);

    // Deriva el nombre: si la ruta acaba en /.conf (DC_tlt), el nombre es el
    // componente penúltimo; si no (DM_tlt), el último componente.
    const bool isConf = cleaned.endsWith(QLatin1String("/.conf"));
    const QString dir = isConf ? cleaned.chopped(QStringLiteral("/.conf").size())
                               : cleaned;
    const int slash = dir.lastIndexOf(QLatin1Char('/'));
    const QString name = (slash >= 0) ? dir.mid(slash + 1) : dir;
    if (name.isEmpty())
        return nullptr;

    const Topic *topic = findByName(name);
    if (topic == nullptr)
        return nullptr;

    // Validación opcional contra la raíz del idioma: el nombre debe casar con
    // la ruta real derivada por AppPaths.
    if (!topicsRoot.isEmpty()) {
        const QString expectedData = QDir::cleanPath(
            topicsRoot + QLatin1Char('/') + name);
        const QString expectedConf = expectedData + QStringLiteral("/.conf");
        if (cleaned != expectedData && cleaned != expectedConf)
            return nullptr;
    }
    return topic;
}

} // namespace core
} // namespace idiomind