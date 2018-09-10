#include "vars_statics.h"

#include <QDialog>

#include <iostream>


using namespace std;


namespace ivar {

    QString user_name() {
        QString name = qgetenv("USER");
      if (name.isEmpty())
          name = qgetenv("USERNAME");
      return name;
    }

    QString Username = user_name();

    QString Home = QDir::homePath();


#if (defined (_WIN32) || defined (_WIN64))
    QString DT = Home+"/AppData/Local/Temp/.idiomind-"+Username;
    QString DS = "C:/Program Files/Idiomind";
    QString DS_a = "C:/Program Files/Idiomind/addons";
    QString DM = Home+"/AppData/Local/Idiomind";
    QString DM_t = Home+"/AppData/Local/Idiomind/Topics";
    QString DC = Home+"/AppData/Local/Idiomind/Config/";
    QString DC_a =  Home+"AppData/Local/Idiomind/Config/addons";
    QString DC_d =  Home+"AppData/Local/Idiomind/Config/addons/dict/enables";
    QString FILE_mn = Home+"/AppData/Local/Idiomind/Config/tpc";
    QString FILE_conf = Home+"/AppData/Local/Idiomind/Config/config.cfg";
#endif


#if (defined (LINUX) || defined (__linux__))
    QString DT = "/tmp/.idiomind-"+Username+"/";
    QString DS = "/usr/share/idiomind/";
    QString DS_a = "/usr/share/idiomind/addons/";
    QString DM = Home+"/.idiomind/";
    QString DM_t = Home+"/.idiomind/topics/";
    QString DC = Home+"/.config/idiomind/";
    QString DC_a =  Home+"/.config/idiomind/addons/";
    QString DC_d =  Home+"/.config/idiomind/addons/dict/enables/";
    QString FILE_mn = Home+"/.config/idiomind/tpc";
    QString FILE_conf = Home+"/.config/idiomind/config.cfg";
#endif


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

Session::Session(QObject *parent) : QObject(parent)
{
    if (!QDir(ivar::DM).exists() || !QDir(ivar::DC).exists()) {

        Welcome newu;
        newu.show();
    }

}


