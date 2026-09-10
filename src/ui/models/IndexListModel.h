// F6-B.2 - IndexListModel: modelo para ítems del índice de aprendizaje.
// Autoridad: items_list.sh notebook_1 tab 1, tls.sh colorize (3 líneas/ítem).
//
// Lee QStringList de TopicDataRepository::loadIndex (3 líneas/ítem:
// text[+html], chk TRUE/FALSE, srce) y expone roles para QML.
#pragma once

#include <QAbstractListModel>
#include <QList>
#include <QString>

namespace idiomind {
namespace ui {

struct IndexItem {
    QString text;   // puede contener HTML spans
    QString plain;  // texto sin HTML
    bool checked;   // TRUE/FALSE del chk
    QString srce;   // traducción
};

class IndexListModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum Roles {
        TextRole = Qt::UserRole + 1,
        PlainTextRole,
        CheckedRole,
        SrceRole,
    };
    Q_ENUM(Roles)

    explicit IndexListModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    // Carga desde la QStringList de loadIndex (3 líneas/ítem).
    void loadFromIndexLines(const QStringList &lines);

    void clear();

    int itemCount() const { return m_items.size(); }

private:
    QList<IndexItem> m_items;

    // Elimina tags HTML de un string.
    static QString stripHtml(const QString &html);
};

} // namespace ui
} // namespace idiomind
