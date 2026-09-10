// F5 - ITtsProvider: interfaz para síntesis de voz.
#pragma once
#include <QDir>
#include <QString>

namespace idiomind {
namespace services {

class ITtsProvider {
public:
    virtual ~ITtsProvider() = default;
    // Sintetiza texto → archivo MP3 en outDir. Devuelve nombre del fichero.
    virtual bool synthesize(const QString &text, const QString &lang,
                            const QDir &outDir, QString *outFile) = 0;
};

} // namespace services
} // namespace idiomind
