// F2.2 - TopicList: colección ordenada y determinista de Topic (dominio puro).
// Autoridad: docs/09-qt-architecture.md §3.8, mngr.sh mkmn / add.sh new_topic.
//
// NOTA: NO es un QAbstractListModel (F2.2 prohibe UI/QML). Es el contenedor de
// dominio:
//  - el ORDEN lo impone TopicRepository::listTopics() replicando `find ls -tNd`
//    de mkmn (requiere mtime del filesystem, no hay I/O en core);
//  - aquí solo se preserva el orden de inserción (determinista) y se ofrecen
//    las operaciones del agregado: add/remove/findByName/findByPath/contains.
// Topic no conoce su path (el repositorio aplica AppPaths); por eso
// findByPath necesita la raíz del idioma (DM_tl) para derivar el nombre.
#pragma once

#include "core/topic/Topic.h"

#include <QList>
#include <QString>

namespace idiomind {
namespace core {

class TopicList
{
public:
    TopicList() = default;

    // Inserta al final. Devuelve false si ya existe un Topic con el mismo
    // nombre (el original no admite duplicados: new_topic deduplica con
    // sufijos " (n)" y mkmn lista directorios únicos).
    bool add(const Topic &topic);

    // Elimina por nombre. Devuelve false si el nombre no estaba presente.
    bool remove(const QString &name);
    void clear() { m_topics.clear(); }

    bool contains(const QString &name) const { return findByName(name) != nullptr; }
    bool contains(const Topic &topic) const { return findByName(topic.name()) != nullptr; }
    int count() const { return m_topics.size(); }
    bool isEmpty() const { return m_topics.isEmpty(); }

    Topic &at(int i) { return m_topics[i]; }
    const Topic &at(int i) const { return m_topics.at(i); }

    // Busca por nombre exacto (los nombres de topic pueden tener espacios y
    // unicode). Devuelve nullptr si no existe.
    Topic *findByName(const QString &name);
    const Topic *findByName(const QString &name) const;

    // Busca por directorio del topic (DM_tlt, o su .conf DC_tlt). `topicsRoot`
    // es la raíz del idioma ($DM_tl = .../topics/<tlng>); se usa para validar
    // que `topicPath` coincide con el directorio esperado para ese nombre.
    Topic *findByPath(const QString &topicPath, const QString &topicsRoot = QString());
    const Topic *findByPath(const QString &topicPath,
                            const QString &topicsRoot = QString()) const;

    const QList<Topic> &topics() const { return m_topics; }

    bool operator==(const TopicList &o) const { return m_topics == o.m_topics; }

private:
    QList<Topic> m_topics;
};

} // namespace core
} // namespace idiomind