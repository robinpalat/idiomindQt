// F6-B.1 - TopicListModel: QAbstractListModel para lista de Topics.
// Autoridad: doc 09 §6.1 (IndexListModel), chng.sh (topic selector).
//
// Expone TopicList a QML con roles: name, stts, sttsLabel, sttsColor.
// NO contiene lógica de negocio; solo adapta TopicList a QML.
#pragma once

#include "core/topic/Topic.h"
#include "core/topic/TopicList.h"

#include <QAbstractListModel>

namespace idiomind {
namespace ui {

class TopicListModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum Roles {
        NameRole = Qt::UserRole + 1,
        SttsRole,
        SttsLabelRole,
        SttsColorRole,
    };
    Q_ENUM(Roles)

    explicit TopicListModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    // Actualiza el modelo completo desde un TopicList.
    void setTopics(const core::TopicList &topics);

    // Nombre del topic en la posición i.
    QString topicName(int i) const;

    int count() const { return m_topics.size(); }

private:
    QList<core::Topic> m_topics;

    static QString sttsLabel(const QString &stts);
    static QString sttsColor(const QString &stts);
};

} // namespace ui
} // namespace idiomind
