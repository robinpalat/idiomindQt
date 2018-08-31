#include "icontray.h"
#include <string>
#include <iostream>
#include <QMessageBox>
#include <QtWidgets/QtWidgets>
#include <QPixmap>
#include <QIcon>
#include <QSql>
#include "vars_statics.h"
#include "item_list.h"
#include "dlg_add.h"


Icontray::Icontray(QWidget *parent)
    : QMainWindow(parent)
{

    watcher = new QFileSystemWatcher();
    connect(watcher, SIGNAL(fileChanged(QString)), this, SLOT(fileChangedEvent(QString)));
    watcher->addPath(ivar::FILE_mn);

}

Icontray::~Icontray()
{
    QCoreApplication::quit();
}


void Icontray::fileChangedEvent(const QString & path)
{

  icontray();
}

void Icontray::icontray() {

    Global mGlobal;
    QString tpc = mGlobal.get_textline(ivar::FILE_mn);

    trayIcon = new QSystemTrayIcon(this);

    trayIcon->setIcon(QIcon(ivar::DS+"/images/logo.png"));
    trayIcon->setToolTip("");

    QMenu * menu = new QMenu(this);
    QAction * play_ = new QAction(tr("Play"), this);
    QAction * add_ = new QAction(tr("Add"), this);
    QAction * viewTopic = new QAction(tpc, this);

    QAction * viewTopics = new QAction(tr("Index"), this);
    QAction * quitAction = new QAction(tr("Salir"), this);

    connect(play_, SIGNAL(triggered()), this, SLOT(show_tpc()));
    connect(add_, SIGNAL(triggered()), this, SLOT(show_dlg_add()));
    connect(viewTopic, SIGNAL(triggered()), this, SLOT(show_tpc()));

    // ----------------------------------------
    //viewTopic->setIcon(QIcon("/usr/share/icons/hicolor/16x16/emblems/gtg-home.png"));
    // QIcon::setThemeName("oxygen");
    //viewTopic->setIcon(QIcon(QStyle::SP_MediaPlay));
    // QIcon genericIcon = QIcon::fromTheme(QIcon::themeName());
   //  qDebug() << QIcon::themeName();
    // viewTopic->setIcon(genericIcon);
    //QIcon::setThemeName("elementary");
    //QIcon testicon = QIcon::fromTheme("go-home");
    //viewTopic->setIcon(style()->standardIcon(QStyle::SP_DirHomeIcon));
    // ----------------------------------------

    QIcon testicon;
    QString themes[3]= {"hicolor","elementary","elementary"};
    for (int i = 0; i < 2; i++)
    {
        QIcon::setThemeName(themes[i]);
        testicon = QIcon::fromTheme("go-home");
        if (!testicon.isNull()) break;
    }
    if (testicon.isNull()) testicon = QIcon(ivar::DS+"/images/home.png");

    viewTopic->setIcon(testicon);

    connect(viewTopics, SIGNAL(triggered()), this, SLOT(show_index()));
    connect(quitAction, SIGNAL(triggered()), this, SLOT(close()));

    menu->addAction(add_);
    menu->addAction(play_);

    menu->addAction(viewTopic);
    menu->addSeparator();
    menu->addAction(viewTopics);
    menu->addSeparator();
    menu->addAction(quitAction);

    trayIcon->setContextMenu(menu);
    trayIcon->show();

    connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
    this, SLOT(iconActivated(QSystemTrayIcon::ActivationReason)));
}


void Icontray::show_index() {
    Topics mTopics;
    mTopics.setModal(true);
    mTopics.exec();
}


void Icontray::show_tpc() {
    item_list mitem_list;
    mitem_list.setModal(true);
    mitem_list.exec();
}

void Icontray::show_dlg_add() {
    Add dlg;
    dlg.setModal(true);
    dlg.exec();
}

void Icontray::iconActivated(QSystemTrayIcon::ActivationReason reason)
{
//    item_list tpcs;

//    switch (reason){
//    case QSystemTrayIcon::DoubleClick:
//        qDebug() << "[[[[[[[[  ** ]]]]]]] ";
//    case QSystemTrayIcon::Trigger:

//            if(!this->isVisible()){
//                tpcs.show();
//            } else {
//                tpcs.hide();
//            }

//        break;
//    case QSystemTrayIcon::Context:
//    //case QSystemTrayIcon::DoubleClick:

//    default:
//        break;
//    }
}