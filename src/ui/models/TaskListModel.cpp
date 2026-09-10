#include "ui/models/TaskListModel.h"

namespace idiomind {
namespace ui {

TaskListModel::TaskListModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

int TaskListModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return m_tasks.size();
}

QVariant TaskListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_tasks.size())
        return {};

    const TaskItem &item = m_tasks.at(index.row());
    switch (role) {
    case TopicRole: return item.topic;
    case TagRole:   return item.tag;
    case TitleRole: return item.title;
    case ColorRole: return item.color;
    case ActionRole: return item.action;
    }
    return {};
}

QHash<int, QByteArray> TaskListModel::roleNames() const
{
    return {
        {TopicRole, "topic"},
        {TagRole, "tag"},
        {TitleRole, "title"},
        {ColorRole, "color"},
        {ActionRole, "action"},
    };
}

void TaskListModel::setTasks(const QList<TaskItem> &tasks)
{
    beginResetModel();
    m_tasks = tasks;
    endResetModel();
    emit tasksChanged();
}

void TaskListModel::removeTask(const QString &topic, const QString &action)
{
    for (int i = 0; i < m_tasks.size(); ++i) {
        if (m_tasks.at(i).topic != topic || m_tasks.at(i).action != action)
            continue;
        beginRemoveRows(QModelIndex(), i, i);
        m_tasks.removeAt(i);
        endRemoveRows();
        emit tasksChanged();
        return;
    }
}

void TaskListModel::clear()
{
    beginResetModel();
    m_tasks.clear();
    endResetModel();
    emit tasksChanged();
}

} // namespace ui
} // namespace idiomind
