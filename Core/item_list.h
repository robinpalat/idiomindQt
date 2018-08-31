#ifndef ITEM_LIST_H
#define ITEM_LIST_H

#include <QDialog>

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
#include <QTableWidgetItem>


#include "dlg_vwr.h"
#include "dlg_topics.h"
#include "vars_statics.h"
#include "vars_session.h"
#include "dlg_share.h"
#include "dlg_edittpc.h"
#include "dlg_translate.h"
#include "practice/dlg_practice.h"
#include "Media/database.h"

using namespace std;

namespace Ui {
class item_list;
}

class item_list : public QDialog
{
    Q_OBJECT

public:
    explicit item_list(QWidget *parent = nullptr);
    ~item_list();


public:
    QSqlQueryModel * modal=new QSqlQueryModel();
    bool mod = false;
    void load_data();
    void icontray();
    void show_main_window();
    std::vector< QString > checked_items;
    QString tpc;
    QString col;
    QString get_tpc();
    void edit_mode();



private slots:
    void on_list_learning_doubleClicked(const QModelIndex &index);
    void on_list_learnt_doubleClicked(const QModelIndex &index);
    void on_list_learning_itemChanged(QTableWidgetItem *item);

    void iconActivated(QSystemTrayIcon::ActivationReason reason);

    void closeEvent(QCloseEvent * event);
    void fileChangedEvent(const QString & path);

    void on_pushButton_play_clicked();
    void on_pushButton_close_clicked();

    void on_pushButton_tabmanage_delete_clicked();
    void on_pushButton_tabmanage_edit_clicked();
    void on_pushButton_tabmanage_trans_clicked();
    void on_pushButton_tabmanage_share_clicked();
    void on_pushButton_tabmanage_markas_clicked();

    void on_pushButton_practice_clicked();

private:
    Ui::item_list *ui;
    Vwr * mVwr;
    Topics * mTopics;
    Share * mShare;
    EditTpc * mEditTpc;
    Translate * mTranslate;
    Practice * mPractice;
    QSystemTrayIcon * trayIcon;
    QFileSystemWatcher * watcher;

};



#endif // ITEM_LIST_H