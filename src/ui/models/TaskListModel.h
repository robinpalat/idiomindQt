// F6-B.5 - TaskListModel: modelo para tareas/sugerencias del sistema.
// Autoridad: main.sh _get_list T1..T10, mngr.sh mkmn, update_lists.sh.
//
// Agrega topics de todas las listas T1..T10 con su tipo y texto descriptivo.
#pragma once

#include <QAbstractListModel>
#include <QList>
#include <QString>

namespace idiomind {
namespace ui {

struct TaskItem {
    QString topic;   // nombre del topic
    QString tag;     // T1, T2, T3, T4, T5, T6, T7, T10
    QString title;   // texto descriptivo
    QString color;   // color del badge
    QString action;  // actividad que debe abrirse al seleccionar
};

class TaskListModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int taskCount READ taskCount NOTIFY tasksChanged)
public:
    enum Roles {
        TopicRole = Qt::UserRole + 1,
        TagRole,
        TitleRole,
        ColorRole,
        ActionRole,
    };
    Q_ENUM(Roles)

    explicit TaskListModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    void setTasks(const QList<TaskItem> &tasks);
    void removeTask(const QString &topic, const QString &action);
    void clear();

    int taskCount() const { return m_tasks.size(); }

signals:
    void tasksChanged();

private:
    QList<TaskItem> m_tasks;
};

} // namespace ui
} // namespace idiomind
