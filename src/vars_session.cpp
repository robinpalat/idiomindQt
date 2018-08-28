#include "vars_session.h"


Global::Global()
{


}

QString Global::get_textline(QString tfile){ // To get topic name and topics status
      QFile file(tfile);
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

QString  get_LANG(QString LANG) {
    std::map<QString,QString>  smaplangs;
    std::map<QString,QString>::iterator  it;
    smaplangs["Afrikaans"]="af";
    smaplangs["Azərbaycanca"]="az";
    smaplangs["Català"]="ca";
    smaplangs["Dansk"]="da";
    smaplangs["Deutsch"]="de";
    smaplangs["English"]="en";
    smaplangs["Español"]="es";
    smaplangs["Filipino"]="fil";
    smaplangs["Français"]="fr";
    smaplangs["Italiano"]="it";

    smaplangs["Kiswahili"]="sw";
    smaplangs["Lietuvių"]="lt";
    smaplangs["Magyar"]="hu";
    smaplangs["Malagasy"]="mg";
    smaplangs["Malti"]="mt";
    smaplangs["Nederlands"]="nl";
    smaplangs["Norsk"]="no";
    smaplangs["Polski"]="pl";
    smaplangs["Português"]="pt";
    smaplangs["Română"]="ro";

    smaplangs["Slovenčina"]="sk";
    smaplangs["Slovenščina"]="sl";
    smaplangs["Soomaali "]="so";
    smaplangs["Suomi"]="fi";
    smaplangs["Svenska"]="sv";
    smaplangs["Tiếng Việt"]="vi";
    smaplangs["Türkçe"]="tr";
    smaplangs["Ελληνικά"]="el";
    smaplangs["Беларуская"]="be";
    smaplangs["Беларуская"]="bg";

    smaplangs["Қазақ Тілі"]="kk";
    smaplangs["Македонски"]="mk";
    smaplangs["Монгол"]="mn";
    smaplangs["Русский"]="ru";
    smaplangs["Српски"]="sr";
    smaplangs["Українська"]="uk";
    smaplangs["Ўзбек"]="uz";
    smaplangs["ქართული"]="ka";
    smaplangs["Հայերեն"]="hy";
    smaplangs["‫עברית‬"]="iw";

    smaplangs["മലയാളം"]="ml";
    smaplangs["한국어"]="ko";
    smaplangs["中文 (香港)"]="zh-HK";
    smaplangs["中文（简体中文）"]="zh-CN";
    smaplangs["中文（繁體中文）"]="zh-TW";
    smaplangs["日本語"]="ja";

    return smaplangs[LANG].toUpper();
}


QString Home = QDir::homePath();
QString FILE_mn = Home+"/.config/idiomind/tpc";

QString tpc = "mn()";

QString tlng = get_tlng();
QString slng = get_slng();
QString Source_LANG =  "Source_"+get_LANG(slng);

QString DM_tl = Home+"/.idiomind/topics/"+tlng;
QString DM_tlt = Home+"/.idiomind/topics/"+tlng+"/"+tpc+"/";
QString DC_tlt = Home+"/.idiomind/topics/"+tlng+"/"+tpc+"/.conf";
QString DC_tls = Home+"/.idiomind/topics/"+tlng+"/.share/";
QString FILE_shrdb = Home+"/.idiomind/topics/"+tlng+"/data/config";
QString FILE_tlngdb = Home+"/.idiomind/topics/"+tlng+"/data/"+tlng+".db";
QString FILE_tpcdb = Home+"/.idiomind/topics/"+tlng+"/"+tpc+"/.conf/tpcdb";



