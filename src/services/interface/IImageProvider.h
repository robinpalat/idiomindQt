// F5 - IImageProvider: interfaz para descarga de imágenes.
#pragma once
#include <QDir>
#include <QString>

namespace idiomind {
namespace services {

class IImageProvider {
public:
    virtual ~IImageProvider() = default;
    // Descarga imagen para una palabra/frag. Devuelve true si OK.
    virtual bool download(const QString &word, const QString &lang,
                          const QDir &outDir) = 0;
};

} // namespace services
} // namespace idiomind
