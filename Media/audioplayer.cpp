#include "Media/audioplayer.h"
#include "Media/database.h"

Audioplayer::Audioplayer(QObject *parent) : QObject(parent)
{

}

void Audioplayer::play(QString clue) {

    trgt = clue;
    Database conn;

    conn.Opendb(DM_tl+"/"+tpc+"/.conf/tpcdb");
    QSqlQuery qry;
    qry.prepare("select * from "+Source_LANG+" where trgt=(:trgt_val)");
    qry.bindValue(":trgt_val", trgt);

    if (qry.exec( )) {
        while(qry.next()) {
            cdid = qry.value(12).toString();
        }
    }
    else {

        qDebug() << qry.lastError();
    }

    QString trgt_mp1 = DC_tls+"audio/"+trgt.toLower()+".mp3";
    QString trgt_mp2 = DM_tl+tpc+"/"+cdid+".mp3";

    QMediaPlayer player;

    if(QFileInfo(trgt_mp1).exists()){
        player.setMedia(QUrl::fromLocalFile(trgt_mp1));
        player.play();
        qDebug() << trgt_mp1 << trgt_mp2;
    }
    else if(QFileInfo(trgt_mp2).exists()){
        player.setMedia(QUrl::fromLocalFile(trgt_mp2));
        player.play();
    }

    qry.finish();
    conn.Closedb();
}
