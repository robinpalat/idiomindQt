#include "vars_session.h"
#include "src/database.h"

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


QString get_tlng(){

    Database conn;
    conn.Opendb(config_db);
    QSqlQuery qry;

    qry.prepare("select tlng from lang");
    if(!qry.exec()) qDebug() << "Error";
    qry.next();
    QString pre_tlng = qry.value(0).toString();

    qry.finish();
    conn.Closedb();
    return pre_tlng;
}

QString get_slng(){
    Database conn;
    conn.Opendb(config_db);
    QSqlQuery qry;

    qry.prepare("select slng from lang");
    if(!qry.exec()) qDebug() << "Error";
    qry.next();
    QString pre_slng = qry.value(0).toString();

    qry.finish();
    conn.Closedb();
    return pre_slng;
}

QString mn(){
    QString Home = QDir::homePath();
      QFile file(FILE_mn);
      QString line;
      if (file.open(QIODevice::ReadOnly)) {
          QTextStream in(&file);
          while (!in.atEnd()) line = in.readLine();
          file.close();
      }
      qDebug() << line;
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


#if (defined (_WIN32) || defined (_WIN64))
    QString FILE_mn = Home+"/AppData/Local/Idiomind/Config/tpc";
    QString config_db = Home+"/AppData/Local/Idiomind/Config/config";
#endif

#if (defined (LINUX) || defined (__linux__))
    QString FILE_mn = Home+"/.config/idiomind/tpc";
    QString config_db = Home+"/.config/idiomind/config";
#endif


QString tpc = mn();
QString tlng = get_tlng();
QString slng = get_slng();
QString Source_LANG =  "Source_"+get_LANG(slng);


#if (defined (_WIN32) || defined (_WIN64))
    QString DM_tl = Home+"/AppData/Local/Idiomind/Topics/"+tlng;
    QString DM_tlt = DM_tl+"/"+tpc+"/";
    QString DC_tlt = DM_tl+"/"+tpc+"/.conf/";
    QString DC_tls = DM_tl+"/.share/";
    QString FILE_shrdb = DC_tls+"/data/config";
    QString FILE_tlngdb = DC_tls+"/data/"+tlng+".db";
    QString FILE_tpcdb =  DC_tlt+"tpcdb";
#endif

#if (defined (LINUX) || defined (__linux__))
    QString DM_tl = Home+"/.idiomind/topics/"+tlng+"/";
    QString DM_tlt = Home+"/.idiomind/topics/"+tlng+"/"+tpc+"/";
    QString DC_tlt = Home+"/.idiomind/topics/"+tlng+"/"+tpc+"/.conf/";
    QString DC_tls = Home+"/.idiomind/topics/"+tlng+"/.share/";
    QString FILE_shrdb = Home+"/.idiomind/topics/"+tlng+"/data/config";
    QString FILE_tlngdb = Home+"/.idiomind/topics/"+tlng+"/data/"+tlng+".db";
    QString FILE_tpcdb = Home+"/.idiomind/topics/"+tlng+"/"+tpc+"/.conf/tpcdb";
#endif
