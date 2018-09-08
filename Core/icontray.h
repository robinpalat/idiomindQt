#ifndef ICONTRAY_H
#define ICONTRAY_H

#include <QMainWindow>
#include <QDebug>
#include <QFileInfo>
#include <QtSql/QtSql>
#include <sys/stat.h>
#include <iostream>
#include <fstream>
#include <QSql>
#include <QCloseEvent>
#include <QSystemTrayIcon>
#include <QAction>

#include <QFileSystemWatcher>

#include "mainwindow.h"
#include "dlg_vwr.h"
#include "dlg_topics.h"
#include "vars_statics.h"
#include "vars_session.h"

class Icontray : public QMainWindow
{
    Q_OBJECT

public:
    Icontray(QWidget *parent = nullptr);
    ~Icontray();

    void icontray();
    bool UpdateFileTimestamp(std::string fileName);

private slots:
    void show_tpc();
    void show_dlg_add();
    void iconActivated(QSystemTrayIcon::ActivationReason reason);
    void show_index();
    void fileChangedEvent(const QString & path);

private:
    Vwr * mVwr;
    Topics * mTopics;
    QSystemTrayIcon * trayIcon;
    MainWindow * mMainWindow;
    QFileSystemWatcher * watcher;

};

#endif // ICONTRAY_H
