#include "ui/models/IndexListModel.h"

#include <QRegularExpression>

namespace idiomind {
namespace ui {

IndexListModel::IndexListModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

int IndexListModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return m_items.size();
}

QVariant IndexListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_items.size())
        return {};

    const IndexItem &item = m_items.at(index.row());
    switch (role) {
    case TextRole:      return item.text;
    case PlainTextRole: return item.plain;
    case CheckedRole:   return item.checked;
    case SrceRole:      return item.srce;
    }
    return {};
}

QHash<int, QByteArray> IndexListModel::roleNames() const
{
    return {
        {TextRole, "text"},
        {PlainTextRole, "plainText"},
        {CheckedRole, "checked"},
        {SrceRole, "srce"},
    };
}

void IndexListModel::loadFromIndexLines(const QStringList &lines)
{
    beginResetModel();
    m_items.clear();

    // 3 líneas por ítem: text, chk, srce
    for (int i = 0; i + 2 < lines.size(); i += 3) {
        const QString textLine = lines.at(i).trimmed();
        const QString chkLine = lines.at(i + 1).trimmed();
        const QString srceLine = lines.at(i + 2).trimmed();

        if (textLine.isEmpty())
            continue;

        IndexItem item;
        item.text = textLine;
        item.plain = stripHtml(textLine);
        item.checked = (chkLine == QLatin1String("TRUE"));
        item.srce = srceLine;
        m_items.append(item);
    }

    endResetModel();
}

void IndexListModel::clear()
{
    beginResetModel();
    m_items.clear();
    endResetModel();
}

QString IndexListModel::stripHtml(const QString &html)
{
    static QRegularExpression tagRe(QStringLiteral("<[^>]*>"));
    QString result = html;
    result.remove(tagRe);
    return result.trimmed();
}

} // namespace ui
} // namespace idiomind
