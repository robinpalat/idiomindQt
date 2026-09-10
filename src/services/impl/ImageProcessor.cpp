#include "services/impl/ImageProcessor.h"

#include <QPainter>

namespace idiomind {
namespace services {

bool ImageProcessor::resizeToCard(const QImage &in, const QString &outPath) const
{
    if (in.isNull())
        return false;
    // Params del Bash: 400x270, quality 90
    const int targetW = 400;
    const int targetH = 270;
    QImage out(targetW, targetH, QImage::Format_RGB32);
    out.fill(Qt::white);

    // Scale to fill, then center-crop
    const double ratio = qMax(static_cast<double>(targetW) / in.width(),
                               static_cast<double>(targetH) / in.height());
    const int scaledW = static_cast<int>(in.width() * ratio);
    const int scaledH = static_cast<int>(in.height() * ratio);
    const QImage scaled = in.scaled(scaledW, scaledH, Qt::IgnoreAspectRatio,
                                    Qt::SmoothTransformation);

    const int x = (scaledW - targetW) / 2;
    const int y = (scaledH - targetH) / 2;
    QPainter p(&out);
    p.drawImage(-x, -y, scaled);
    p.end();

    return out.save(outPath, "JPEG", 90);
}

bool ImageProcessor::createThumbnail(const QImage &in, const QString &outPath) const
{
    if (in.isNull())
        return false;
    // Params del Bash: 405x275^ (crop), fondo blanco si h*100/w > 80
    const int thumbW = 405;
    const int thumbH = 275;
    QImage out(thumbW, thumbH, QImage::Format_RGB32);
    out.fill(Qt::white);

    // Check aspect ratio (Bash: h*100/w > 80 → fondo blanco)
    const bool useWhiteBg = (in.height() * 100 / in.width()) > 80;

    double ratio;
    if (useWhiteBg) {
        ratio = static_cast<double>(thumbH) / in.height();
    } else {
        ratio = qMax(static_cast<double>(thumbW) / in.width(),
                      static_cast<double>(thumbH) / in.height());
    }

    const int scaledW = static_cast<int>(in.width() * ratio);
    const int scaledH = static_cast<int>(in.height() * ratio);
    const QImage scaled = in.scaled(scaledW, scaledH, Qt::IgnoreAspectRatio,
                                    Qt::SmoothTransformation);

    const int x = (scaledW - thumbW) / 2;
    const int y = (scaledH - thumbH) / 2;
    QPainter p(&out);
    p.drawImage(-x, -y, scaled);
    p.end();

    return out.save(outPath, "JPEG", 90);
}

} // namespace services
} // namespace idiomind
