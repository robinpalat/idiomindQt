﻿#include "icontray.h"
#include <string>
#include <iostream>
#include <QMessageBox>
#include <QtWidgets/QtWidgets>
#include <QPixmap>
#include <QIcon>
#include <QSql>
#include "vars_statics.h"
#include "dlg_add.h"



Icontray::Icontray(QWidget *parent)
    : QMainWindow(parent) {

    watcher = new QFileSystemWatcher();
    connect(watcher, SIGNAL(fileChanged(QString)), this, SLOT(fileChangedEvent(QString)));
    watcher->addPath(ivar::FILE_mn);
}


Icontray::~Icontray() {
    QCoreApplication::quit();
}


void Icontray::fileChangedEvent(const QString) {
    Global mGlobal;
    QString tpc = mGlobal.get_textline(ivar::FILE_mn);
    Icontray::updateTopicLabel(tpc);
}


void Icontray::icontray() {

    Global mGlobal;
    QString tpc = mGlobal.get_textline(ivar::FILE_mn);
    trayIcon = new QSystemTrayIcon(this);
    trayIcon->setIcon(QIcon(ivar::DS+"/images/logo.png"));
    trayIcon->setToolTip("ddddddd");
    QMenu * menu = new QMenu(this);
    QAction * play_ = new QAction(tr("Play"), this);
    QAction * add_ = new QAction(tr("Add"), this);
    QAction * viewTopic = new QAction(tpc, this);
    QAction * viewTopics = new QAction("Index", this);
    QAction * options = new QAction(tr("Options"), this);
    QAction * about = new QAction(tr("About"), this);
    QAction * quitAction = new QAction(tr("Salir"), this);

    connect(add_, SIGNAL(triggered()), this, SLOT(show_dlg_add()));
    connect(play_, SIGNAL(triggered()), this, SLOT(show_tpc()));
    connect(viewTopic, SIGNAL(triggered()), this, SLOT(show_tpc()));
    connect(viewTopics, SIGNAL(triggered()), this, SLOT(show_index()));
    connect(quitAction, SIGNAL(triggered()), this, SLOT(quitar()));
    connect(this, &Icontray::setTopicLabel, viewTopic, &QAction::setText);


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


    menu->addAction(add_);
    menu->addAction(play_);
    menu->addAction(viewTopic);
    menu->addSeparator();
    menu->addAction(viewTopics);
    menu->addSeparator();
    menu->addAction(options);
    menu->addAction(about);
    menu->addAction(quitAction);
    trayIcon->setContextMenu(menu);
    trayIcon->show();


    connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
    this, SLOT(iconActivated(QSystemTrayIcon::ActivationReason)));
}


void Icontray::quitar() {

    QCoreApplication::quit();
}


void Icontray::show_index() {

    Topics mTopics;
    mTopics.setModal(true);
    mTopics.exec();
}


void Icontray::show_tpc() {

    mMainWindow = new MainWindow(this);
    mMainWindow->load_data();
    mMainWindow->show();
}


void Icontray::show_dlg_add() {
    Add dlg;
    dlg.setModal(true);
    dlg.exec();
}


void Icontray::iconActivated(QSystemTrayIcon::ActivationReason reason) {

    mMainWindow = new MainWindow(this);
    switch (reason){
    case QSystemTrayIcon::DoubleClick:
    case QSystemTrayIcon::Trigger:
            if(!this->isVisible()){
                mMainWindow->show();
            } else {
                mMainWindow->show();
            }
        break;
    case QSystemTrayIcon::Context:

    default:
        break;
    }
}




