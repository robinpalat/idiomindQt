#include "vars_session.h"

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
  QString slangs[47][47] = {

      { "", "Afrikaans", "Azərbaycanca","Català","Dansk","Deutsch","English","Español","Filipino","Français","Italiano",
        "Kiswahili","Lietuvių","Magyar","Malagasy","Malti","Nederlands","Norsk","Polski","Português","Română",
        "Slovenčina","Slovenščina","Soomaali","Suomi","Svenska","Tiếng Việt","Türkçe","Ελληνικά","Беларуская","Беларуская",
        "Қазақ Тілі","Македонски","Монгол","Русский","Српски","Українська","Ўзбек","ქართული","Հայերեն","‫עברית‬",
        "മലയാളം","한국어","中文 (香港)","中文（简体中文","中文（繁體中文","日本語" },

      { "", "af","az","ca","da","de","en","es","fil","fr","it",
        "sw","lt","hu","mg","mt","nl","no","pl","pt","ro",
        "sk","sl","so","fi","sv","vi","tr","el","be","bg",
        "kk","mk","mn","ru","sr","uk","uz","ka","hy","iw",
        "ml","ko","zh-HK","zh-CN","zh-TW","ja" }
  };

}

/*
['Afrikaans']='af' \
['Azərbaycanca']='az' \
['Català']='ca' \
['Dansk']='da' \
['Deutsch']='de' \
['English']='en' \
['Español']='es' \
['Filipino']='fil' \
['Français']='fr' \
['Italiano']='it' \

['Kiswahili']='sw' \
['Lietuvių']='lt' \
['Magyar']='hu' \
['Malagasy']='mg' \
['Malti']='mt' \
['Nederlands']='nl' \
['Norsk']='no' \
['Polski']='pl' \
['Português']='pt' \
['Română']='ro' \

['Slovenčina']='sk' \
['Slovenščina']='sl' \
['Soomaali ']='so' \
['Suomi']='fi' \
['Svenska']='sv' \
['Tiếng Việt']='vi' \
['Türkçe']='tr' \
['Ελληνικά']='el' \
['Беларуская']='be' \
['Беларуская']='bg' \

['Қазақ Тілі']='kk' \
['Македонски']='mk' \
['Монгол']='mn' \
['Русский']='ru' \
['Српски']='sr' \
['Українська']='uk' \
['Ўзбек']='uz' \
['ქართული']='ka' \
['Հայերեն']='hy' \
['‫עברית‬']='iw' \

['മലയാളം']='ml' \
['한국어']='ko' \
['中文 (香港)']='zh-HK' \
['中文（简体中文）']='zh-CN' \
['中文（繁體中文）']='zh-TW' \
['日本語']='ja' )
*/


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
