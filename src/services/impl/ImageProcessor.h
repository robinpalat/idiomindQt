// F5 - ImageProcessor: resize/crop de imágenes.
// Autoridad: Dev/ifs/mods/add/add.sh img_word() (imagemagick params).
//
// Resize: 400x270, thumb: 405x275^, quality 90, fondo blanco si h*100/w>80.
#pragma once

#include <QImage>
#include <QString>

namespace idiomind {
namespace services {

class ImageProcessor
{
public:
    // Resize/crop centrado a 400x270 (card format).
    bool resizeToCard(const QImage &in, const QString &outPath) const;

    // Genera thumbnail 405x275^ con fondo blanco si aspect ratio lo requiere.
    bool createThumbnail(const QImage &in, const QString &outPath) const;
};

} // namespace services
} // namespace idiomind
