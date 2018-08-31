#ifndef AUDIOPLAYER_H
#define AUDIOPLAYER_H

#include <QObject>
#include <QMediaPlayer>

#include "Core/vars_statics.h"
#include "Core/vars_session.h"

class Audioplayer : public QObject
{
    Q_OBJECT
public:
    explicit Audioplayer(QObject *parent = nullptr);

    void play(QString path);
    QString trgt, cdid;

signals:

public slots:

private:
    //QMediaPlayer * player;
};

#endif // AUDIOPLAYER_H
