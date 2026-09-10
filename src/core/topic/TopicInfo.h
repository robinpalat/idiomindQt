// F2.1 - TopicInfo: metadata de un topic.
// Autoridad: docs/05-topic-format.md, default/vars línea 3, default/sets.cfg tsets.
#pragma once

#include <QJsonObject>
#include <QString>

namespace idiomind {
namespace core {

// Campos de metadata del topic (orden tsets / vars línea 3). Todos string,
// igual que el original: nsze se conserva sin convertir a double (p.ej. "8,6M",
// "2.2M"), stts y los contadores se exportan como string.
class TopicInfo {
public:
    static constexpr int fieldCount() { return 19; }
    static const char *fieldName(int i);  // nombre tsets por índice 0..18

    TopicInfo() = default;

    const QString &name() const { return m_name; }
    const QString &slng() const { return m_slng; }
    const QString &tlng() const { return m_tlng; }
    const QString &autr() const { return m_autr; }
    const QString &cntt() const { return m_cntt; }
    const QString &ctgy() const { return m_ctgy; }
    const QString &ilnk() const { return m_ilnk; }
    const QString &orig() const { return m_orig; }
    const QString &dtec() const { return m_dtec; }
    const QString &dteu() const { return m_dteu; }
    const QString &dtei() const { return m_dtei; }
    const QString &nwrd() const { return m_nwrd; }
    const QString &nsnt() const { return m_nsnt; }
    const QString &nimg() const { return m_nimg; }
    const QString &naud() const { return m_naud; }
    const QString &nsze() const { return m_nsze; }
    const QString &levl() const { return m_levl; }
    const QString &info() const { return m_info; }
    const QString &stts() const { return m_stts; }

    void setName(const QString &v) { m_name = v; }
    void setSlng(const QString &v) { m_slng = v; }
    void setTlng(const QString &v) { m_tlng = v; }
    void setAutr(const QString &v) { m_autr = v; }
    void setCntt(const QString &v) { m_cntt = v; }
    void setCtgy(const QString &v) { m_ctgy = v; }
    void setIlnk(const QString &v) { m_ilnk = v; }
    void setOrig(const QString &v) { m_orig = v; }
    void setDtec(const QString &v) { m_dtec = v; }
    void setDteu(const QString &v) { m_dteu = v; }
    void setDtei(const QString &v) { m_dtei = v; }
    void setNwrd(const QString &v) { m_nwrd = v; }
    void setNsnt(const QString &v) { m_nsnt = v; }
    void setNimg(const QString &v) { m_nimg = v; }
    void setNaud(const QString &v) { m_naud = v; }
    void setNsze(const QString &v) { m_nsze = v; }
    void setLevl(const QString &v) { m_levl = v; }
    void setInfo(const QString &v) { m_info = v; }
    void setStts(const QString &v) { m_stts = v; }

    // Objeto JSON en el orden tsets (name..stts). Los valores se emiten como
    // strings (nunca se convierten a número), igual que el formato.
    QJsonObject toJson() const;
    // Forma textual ordenada de la 3ª línea (`"name":...,"stts":"0"` a partir
    // de la 2ª clave, sin llaves de cierre). QJsonDocument reordena claves al
    // serializar; el .idmnd requiere el orden tsets exacto.
    QString toJsonText() const;
    static TopicInfo fromJson(const QJsonObject &obj);
    // Parsea el segmento de texto de la 3ª línea, p.ej. `"name":...,"stts":"0"`.
    // Acepta tanto el objeto completo `{...}` como la cola sin llaves.
    static TopicInfo fromJsonText(const QString &text);

    bool operator==(const TopicInfo &o) const;

private:
    QString m_name, m_slng, m_tlng, m_autr, m_cntt, m_ctgy, m_ilnk,
        m_orig, m_dtec, m_dteu, m_dtei, m_nwrd, m_nsnt, m_nimg,
        m_naud, m_nsze, m_levl, m_info, m_stts;
};

} // namespace core
} // namespace idiomind

Q_DECLARE_METATYPE(idiomind::core::TopicInfo)