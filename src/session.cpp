#include "session.h"

#include <QDialog>

using namespace std;


namespace ivar {

    QString mn(){
    QString Home = QDir::homePath();
    QString FILE_mn = Home+"/.config/idiomind/tpc";
      QFile file(FILE_mn);
      QString line;
      if (file.open(QIODevice::ReadOnly)) {
          QTextStream in(&file);
          while (!in.atEnd()) line = in.readLine();
          file.close();
      }
      return line;
    }

    QString user_name() {
        QString name = qgetenv("USER");
      if (name.isEmpty())
          name = qgetenv("USERNAME");
      return name;
    }

    void connClose()
    {
      QSqlDatabase mydb;
      QSqlQuery qry;
      qry.finish();
      mydb.close();
      mydb = QSqlDatabase();
      mydb.removeDatabase(QSqlDatabase::defaultConnection);
    }

    bool connOpen()
    {
      QString Home = QDir::homePath();
      QSqlDatabase mydb;
      mydb=QSqlDatabase::addDatabase("QSQLITE");
      mydb.setDatabaseName(Home+"/.config/idiomind/config");
      if (!mydb.open())
      {
          qDebug()<<("Failed to open the database");
          return false;
      }
      else
      {
          return true;
      }
    }

    QString get_tlng(){
      connOpen();
      QSqlQuery qry;
      qry.prepare("select tlng from lang");
      if(!qry.exec()) qDebug() << "Error";
      qry.next();
      QString pre_tlng = qry.value(0).toString();
      qry.finish(); connClose();
      return pre_tlng;
    }

    QString get_slng(){
      connOpen();
      QSqlQuery qry;
      qry.prepare("select slng from lang");
      if(!qry.exec()) qDebug() << "Error";
      qry.next();
      QString pre_slng = qry.value(0).toString();
      qry.finish(); connClose();
      return pre_slng;
    }

  QString Username = user_name();
  QString Home = QDir::homePath();


  QString DT = "/tmp/.idiomind-"+Username;

  QString DS = "/usr/share/idiomind";
  QString DS_a = "/usr/share/idiomind/addons";
  QString FILE_mn = Home+"/.config/idiomind/tpc";

  QString tpc = mn();

  QString tlng = get_tlng();
  QString slng = get_slng();

  QString DM = Home+"/.idiomind";
  QString DM_t = Home+"/.idiomind/topics";
  QString DM_tl = Home+"/.idiomind/topics/"+tlng;
  QString DM_tlt = Home+"/.idiomind/topics/"+tlng+"/"+tpc+"/";
  QString DC_tlt = Home+"/.idiomind/topics/"+tlng+"/"+tpc+"/.conf";

  QString DC_tls = Home+"/.idiomind/topics/"+tlng+"/.share/";

  QString DC = Home+"/.config/idiomind/";
  QString DC_a =  Home+"/.config/idiomind/addons";
  QString DC_d =  Home+"/.config/idiomind/addons/dict/enables";

  QString FILE_shrdb = Home+"/.idiomind/topics/"+tlng+"/data/config";
  QString FILE_tlngdb = Home+"/.idiomind/topics/"+tlng+"/data/"+tlng+".db";
  QString FILE_tpcdb = Home+"/.idiomind/topics/"+tlng+"/"+tpc+"/.conf/tpcbd";
}


Session::Session(QObject *parent) : QObject(parent)
{
    if (!QDir(ivar::DM).exists() || !QDir(ivar::DC).exists()) {

        Welcome newu;
        newu.show();
    }

    out_result();
}


void Session::out_result(){

    cout << "FILE_tlngdb: " << ivar::FILE_tlngdb.toStdString() << endl;
}
