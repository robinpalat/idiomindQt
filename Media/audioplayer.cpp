#include "Media/audioplayer.h"
#include "Media/database.h"

Audioplayer::Audioplayer(QObject *parent) : QObject(parent)
{

}

QString Audioplayer::pathplay(QString clue) {

    Database conn;
    trgt = clue;

    QSqlDatabase db = Database::instance().getConnection(tpc);
    QSqlQuery qry(db);

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
    QString path;

    if(QFileInfo(trgt_mp1).exists()){
        path = trgt_mp1;
    }
    else if(QFileInfo(trgt_mp2).exists()){
        path = trgt_mp1;

    }

    qry.finish();

    return path;
}
