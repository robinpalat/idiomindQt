#pragma once

#include <QString>

namespace idiomind {
namespace core {

/**
 * Resolución de rutas replicando el layout del original (c.conf, doc 02):
 *
 *   dataRoot()   = $HOME/.idiomind                      (DM)
 *   configRoot() = $HOME/.config/idiomind               (DC / DC_s)
 *   topicsDir()  = $HOME/.idiomind/topics              (DM_t)
 *   backupDir()  = $HOME/.idiomind/backup
 *   topicsDirForLanguage(tlng)  = .../topics/<tlng>      (DM_tl)
 *   sharedDirForLanguage(tlng)  = .../topics/<tlng>/.share  (DM_tls)
 *   topicDataDir(tlng, topic)   = .../topics/<tlng>/<topic> (DM_tlt)
 *   topicConfDir(tlng, topic)   = .../topics/<tlng>/<topic>/.conf (DC_tlt)
 *   configDb()    = $DC_s/config                        (cfgdb, doc 04)
 *   sharedDataDir(tlng) = $DM_tls/data
 *   sharedDb(tlng)      = $DM_tls/data/config           (shrdb, doc 04)
 *   tlngDb(tlng)        = $DM_tls/data/<tlng>.db        (tlngdb, doc 04)
 *   topicDb(tlng, topic)        = $DC_tlt/tpc             (tpcdb, doc 04)
 *   tmpDir()      = <tmpRoot>/.idiomind-<user>          (DT, original: /tmp/.idiomind-$USER)
 *
 * DECISIÓN DE DISEÑO — F0:
 *  - AppPaths es SOLO cálculo de rutas: nunca crea directorios ni toca
 *    datos existentes (en el original la creación la hacían check_dir /
 *    check_file en cada script; eso llega en F2 con TopicRepository).
 *  - homeDir y tmpRoot son inyectables para poder testear sobre rutas
 *    temporales sin afectar al filesystem real del usuario.
 *  - El nombre de usuario se toma de $USER (igual que el original); fallback
 *    "user" si la variable no existe. En Windows/macOS se revisará en F8.
 *
 * CORRECCIÓN F2.2 (ground truth = c.conf línea 25):
 *  - topicConfDir EXIGE idioma y topic: en el original
 *    DC_tlt="$HOME/.idiomind/topics/$tlng/$tpc/.conf" (DENTRO del topic,
 *    bajo DM_tl), NO bajo $HOME/.config/idiomind. El diseño F0 lo calculaba
 *    en configRoot y sin idioma; era un error introducido al documentar.
 *    tst_apppaths se actualizó en F2.2 al valor real.
 */
class AppPaths
{
public:
    explicit AppPaths(QString homeDir = defaultHomeDir(),
                      QString tmpRoot = defaultTempRoot());

    // -- raíces --
    QString homeDir() const;       // $HOME
    QString dataRoot() const;      // $HOME/.idiomind             (DM)
    QString configRoot() const;    // $HOME/.config/idiomind      (DC)
    QString topicsDir() const;     // $HOME/.idiomind/topics      (DM_t)
    QString backupDir() const;     // $HOME/.idiomind/backup

    // -- por idioma de aprendizaje --
    // .../topics/<tlng>
    QString topicsDirForLanguage(const QString& tlng) const;  // DM_tl
    // .../topics/<tlng>/.share
    QString sharedDirForLanguage(const QString& tlng) const;  // DM_tls

    // -- por topic --
    QString topicDataDir(const QString& tlng, const QString& topic) const; // DM_tlt
    QString topicConfDir(const QString& tlng, const QString& topic) const; // DC_tlt

    // -- bases de datos (doc 04) --
    QString configDb() const;                        // cfgdb  = $DC_s/config
    QString sharedDataDir(const QString& tlng) const; // $DM_tls/data
    QString sharedDb(const QString& tlng) const;     // shrdb  = $DM_tls/data/config
    QString tlngDb(const QString& tlng) const;       // tlngdb = $DM_tls/data/<tlng>.db
    QString topicDb(const QString& tlng, const QString& topic) const; // tpcdb = $DC_tlt/tpc

    // -- temporal --
    QString tmpDir() const;                          // DT = /tmp/.idiomind-<user>

private:
    static QString defaultHomeDir();
    static QString defaultTempRoot();
    static QString userFromEnvironment();
    static QString join(const QString& base, const QString& child);

    QString m_homeDir;
    QString m_tmpRoot;
    QString m_userName;
};

} // namespace core
} // namespace idiomind