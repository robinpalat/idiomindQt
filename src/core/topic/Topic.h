// F2.2 - Topic: agregado raíz de un topic en memoria.
// Autoridad: docs/09-qt-architecture.md §3.9, docs/05-topic-format.md.
//
// Alcance F2.2: TopicInfo + ítems (línea plana) + nota. El Topic NO conoce
// repositorios ni rutas (el almacenamiento lo aplica TopicRepository). Los
// sub-agregados de fases posteriores (review, config, listas T1..T10, notas
// de aprendizaje, traducciones, indexLines) pertenecen a F3+.
//
// TopicInfo se REUSA de F2.1 (no se crea una estructura paralela): el nombre
// y el stts viven dentro de TopicInfo, igual que en el 3er campo de .idmnd.
#pragma once

#include "core/topic/Item.h"
#include "core/topic/TopicInfo.h"

#include <QList>
#include <QString>

namespace idiomind {
namespace core {

class Topic
{
public:
    Topic() = default;
    explicit Topic(const TopicInfo &info);

    // -- identidad (delegada en TopicInfo) --
    const QString &name() const { return m_info.name(); }
    void setName(const QString &name) { m_info.setName(name); }

    // -- metadata --
    TopicInfo info() const { return m_info; }
    void setInfo(const TopicInfo &info) { m_info = info; }

    // -- estado de learning (stts) --
    // stts se guarda como texto (igual que el original); statusValue()
    // devuelve el entero, y los valores NO numéricos (o vacíos) se mapean a
    // 13 (Corrupted), exactamente como el Bash: `! [[ $stts =~ $numer ]] && stts=13`.
    QString stts() const { return m_info.stts(); }
    void setStts(const QString &stts) { m_info.setStts(stts); }
    int statusValue() const;

    // -- contenido --
    const QList<Item> &items() const { return m_items; }
    void setItems(const QList<Item> &items) { m_items = items; }
    void addItem(const Item &item) { m_items.append(item); }
    bool isEmpty() const { return m_items.isEmpty(); }

    // -- nota del topic --
    // Original: `echo " " > "${DC_tlt}/note"` (chek_topic, tpc.sh). El valor
    // por defecto es un único espacio, no cadena vacía.
    const QString &note() const { return m_note; }
    void setNote(const QString &note) { m_note = note; }

    bool operator==(const Topic &o) const;
    bool operator!=(const Topic &o) const { return !(*this == o); }

private:
    TopicInfo m_info;
    QList<Item> m_items;
    QString m_note = QStringLiteral(" ");
};

} // namespace core
} // namespace idiomind