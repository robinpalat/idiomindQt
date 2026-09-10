#include "ui/models/TopicListModel.h"

namespace idiomind {
namespace ui {

TopicListModel::TopicListModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

int TopicListModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return m_topics.size();
}

QVariant TopicListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_topics.size())
        return {};

    const core::Topic &topic = m_topics.at(index.row());
    switch (role) {
    case NameRole:     return topic.name();
    case SttsRole:     return topic.stts();
    case SttsLabelRole: return sttsLabel(topic.stts());
    case SttsColorRole: return sttsColor(topic.stts());
    }
    return {};
}

QHash<int, QByteArray> TopicListModel::roleNames() const
{
    return {
        {NameRole, "name"},
        {SttsRole, "stts"},
        {SttsLabelRole, "sttsLabel"},
        {SttsColorRole, "sttsColor"},
    };
}

void TopicListModel::setTopics(const core::TopicList &topics)
{
    beginResetModel();
    m_topics = topics.topics();
    endResetModel();
}

QString TopicListModel::topicName(int i) const
{
    if (i < 0 || i >= m_topics.size())
        return {};
    return m_topics.at(i).name();
}

QString TopicListModel::sttsLabel(const QString &stts)
{
    // Replica de la lógica del original: número de estado
    return stts;
}

QString TopicListModel::sttsColor(const QString &stts)
{
    const int v = stts.toInt();
    switch (v) {
    case 0:  return QStringLiteral("#999999");  // paused
    case 1:  return QStringLiteral("#4CAF50");  // learning
    case 2:  return QStringLiteral("#2196F3");  // mastered
    case 3:  return QStringLiteral("#FF9800");  // waiting
    case 4:  return QStringLiteral("#FF9800");  // waiting alt
    case 5:  return QStringLiteral("#FF5722");  // mid-review
    case 6:  return QStringLiteral("#FF5722");  // mid-review alt
    case 7:  return QStringLiteral("#9C27B0");  // ready
    case 8:  return QStringLiteral("#9C27B0");  // ready alt
    case 9:  return QStringLiteral("#F44336");  // overdue
    case 10: return QStringLiteral("#F44336");  // overdue alt
    default: return QStringLiteral("#999999");  // corrupted
    }
}

} // namespace ui
} // namespace idiomind
