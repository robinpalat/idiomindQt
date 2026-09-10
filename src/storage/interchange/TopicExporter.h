// F3.2 - TopicExporter: exporta un topic del perfil al formato de intercambio
// `.idmnd` legacy (JSON plano de 3 líneas), replicando
// ifs/mods/export/Idiomind Topic (idmnd).sh (sin multimedia).
//
// Fidelidad con el Bash (verificado contra el script):
//  - head: name desde el directorio del topic; dteu = HOY
//    (`date +%F`); slng/tlng con la primera letra en mayúscula (`${slng^}`);
//    orig = nombre del topic; ilnk desde la columna id.ilnk (si vacía,
//    fallback slug(name[:15]) + md5(data)[:20]);
//  - defaults de id (columnas vacías): autr="", nwrd=nsnt=naud=nimg=0,
//    nsze="", stts=0;
//  - info = transcode de $DC_tlt/note (drop líneas vacías, unión con
//    `<br><br>`, & -> &amp;, " -> \", / -> \/) EXACTAMENTE en ese orden;
//  - ítems: una línea plana por ítem (los de trgt vacío se omiten); type==1
//    -> imag (2: imagen propia, 1: imagen compartida, 0: ninguna) e
//    imgr = ${trgt,}; type!=1 -> imag=0 e imgr="";
//  - el objeto JSON del ítem usa el template ACTUAL de default/vars línea 1
//    (23 claves: sin slvi); el resto = get_item (srce, exmp, defn, note,
//    wrds, grmr, mark, link, tags, refr, cdid).
//
// Desviación documentada: los valores del head se emiten con escaping JSON
// estándar (el Bash los inserta sin escapar); para valores sin comillas
// (los típicos del formato) el resultado es idéntico.
#pragma once

#include "core/config/AppPaths.h"

#include <QByteArray>
#include <QString>

namespace idiomind {
namespace storage {

class TopicExporter
{
public:
    explicit TopicExporter(core::AppPaths appPaths);

    // Genera el .idmnd 3-líneas del topic. `ok` = false si el topic no tiene
    // `.conf/data` (el Bash emite un fichero vacío + cabecera en ese caso;
    // aquí se prefiere fallar: la cabecera citaría campos de id ausentes).
    QByteArray exportTopic(const QString &tlng, const QString &topic,
                           bool *ok = nullptr, QString *error = nullptr) const;

private:
    static QString transcodeNote(const QString &note);
    static QString ilnkFallback(const QString &topic, const QByteArray &dataMd5);

    core::AppPaths m_appPaths;
};

} // namespace storage
} // namespace idiomind