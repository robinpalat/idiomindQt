#include "item_list.h"
#include "build/ui/ui_item_list.h"
#include "vars_statics.h"


#include <string>
#include <iostream>
#include <QMessageBox>
#include <QtWidgets/QtWidgets>
#include <QPixmap>
#include <QIcon>
#include <QSql>


// Used to Check if file does exists
#include <QFileInfo>


item_list::item_list(QWidget *parent) : QDialog(parent), ui(new Ui::item_list)
{

    ui->setupUi(this);
    this->hide();
    this->setWindowIcon(QIcon(ivar::DS+"/images/logo.png"));
//    ui->list_learning->setStyleSheet("QTableWidget::item { padding: 20px }");
//    ui->list_learnt->setStyleSheet("QTableWidget::item { padding: 20px }");

    QSqlQuery qry;

    ui->label_tpc_name->setText(tpc);
    this->setWindowTitle("Idiomind - "+tpc);

    watcher = new QFileSystemWatcher();
    connect(watcher, SIGNAL(fileChanged(QString)), this, SLOT(fileChangedEvent(QString)));
    watcher->addPath(ivar::FILE_mn);

    load_data();
}



void item_list::fileChangedEvent(const QString & path)
{
    qDebug() << path;
    load_data();
}


item_list::~item_list()
{
    delete ui;
}


void item_list::show_main_window() {

    if(!this->isVisible()){
        this->show();
    } else {
        this->hide();
    }
}


QString item_list::get_tpc() {
    Global mGlobal;
    return  mGlobal.get_textline(ivar::FILE_mn);
}


// laod list of items
void item_list::load_data() {

    tpc = get_tpc();
    ui->label_tpc_name->setText(tpc);
    this->setWindowTitle("Idiomind - "+tpc);

    QSqlDatabase db = Database::instance().getConnection(tpc);

     // Tab 2
    QSqlQuery qry_a(db);
    qry_a.prepare("select list from learning");
    ui->list_learning->setColumnCount(2);
    ui->list_learning->setRowCount(0);

    int numberOfRows = 0;
    if (qry_a.exec( )) {
        while(qry_a.next()) {
            QString srce = qry_a.value(0).toString();
            QTableWidgetItem *item2 = new QTableWidgetItem("");
            item2->setCheckState(Qt::Unchecked);
            ui->list_learning->insertRow(ui->list_learning->rowCount());
            ui->list_learning->setItem(ui->list_learning->rowCount() -1, 0, new QTableWidgetItem(srce));
            ui->list_learning->setItem(ui->list_learning->rowCount() -1, 1, item2);
            numberOfRows ++;
        }
    }
    ui->tabWidget->setTabText(0, "Learning (" +QString::number(numberOfRows)+ ")");

    // Tab 2
    QSqlQueryModel * tab2_modal=new QSqlQueryModel();

    //QSqlQuery qry_b;
    qry_a.prepare("select list from learnt");
    qry_a.exec();
    tab2_modal->setQuery(qry_a);

    ui->list_learnt->setModel(tab2_modal);

    numberOfRows = 0;
    if(qry_a.last())
    {
        numberOfRows =  qry_a.at() + 1;
        qry_a.first();
        qry_a.previous();
    }

    ui->tabWidget->setTabText(1, "Learnt (" +QString::number(numberOfRows)+ ")");

    qry_a.finish();



    // load note
    QFile file(DM_tl+"/"+tpc+"/.conf/note");
    if(!file.open(QIODevice::ReadOnly)) {
        QMessageBox::information(nullptr, "error", file.errorString());
    }
    QTextStream in(&file);
    QString text = in.readAll();
    ui->textEdit->setHtml(text);
}



void item_list::on_pushButton_play_clicked()
{
    load_data();
    qDebug() << (modal->rowCount());
}


void item_list::on_pushButton_close_clicked()
{
    this->close();
}


void item_list::on_list_learning_doubleClicked(const QModelIndex &index)
{
    QString trgt=ui->list_learning->model()->data(index).toString();

    if (trgt == "") {
        load_data();
        QString trgt=ui->list_learning->model()->data(index).toString();
    }

    mVwr = new Vwr(this);

    mVwr->load_array(trgt, "learning");

    mVwr->show();
}

void item_list::on_list_learnt_doubleClicked(const QModelIndex &index)
{

    QString trgt=ui->list_learnt->model()->data(index).toString();

    if (trgt == "") {
        load_data();
        QString trgt=ui->list_learning->model()->data(index).toString();
    }

    mVwr = new Vwr(this);

    mVwr->load_array(trgt, "learnt");

    mVwr->show();
}


void item_list::iconActivated(QSystemTrayIcon::ActivationReason reason)
{
    switch (reason){
    case QSystemTrayIcon::Trigger:

            if(!this->isVisible()){
                this->show();
            } else {
                this->hide();
            }

        break;
    default:
        break;
    }
}

void item_list::on_list_learning_itemChanged(QTableWidgetItem *item)
{
    if(item->checkState()) {
        QString itm = ui->list_learning->item(item->row(), 0)->text();
        checked_items.push_back(itm);
    }
    else if (!item->checkState()) {
        QString itm = ui->list_learning->item(item->row(), 0)->text();
        checked_items.erase(std::remove(checked_items.begin(), checked_items.end(), itm), checked_items.end());
    }
}

void item_list::closeEvent(QCloseEvent * event)
{
    // Save data
    tpc = get_tpc();

    QSqlDatabase db = Database::instance().getConnection(tpc);
    QSqlQuery qry(db);

    for(std::vector<QString>::iterator chkd = checked_items.begin(); chkd != checked_items.end(); ++chkd) {
       QString checked = *chkd;
       qry.prepare("DELETE FROM learning WHERE list = ?");
       qry.addBindValue(checked);
       if (!qry.exec()) qDebug() << qry.lastError().text();
       qry.prepare("INSERT INTO learnt(list) values(:checked)");
       qry.bindValue(":checked",checked);
       if (!qry.exec()) qDebug() << qry.lastError().text();
    }

    qry.finish();

    // save note
    QString text = ui->textEdit->toHtml();
    QString filename=DM_tl+"/"+tpc+"/.conf/note";
    QFile file( filename );
    if ( file.open(QIODevice::ReadWrite | QIODevice::Truncate | QIODevice::Text))
    {
        QTextStream stream( &file );
        stream << text;
    }

    if(this->isVisible()){
        event->ignore();
        this->hide();
    }
}



void item_list::on_pushButton_tabmanage_delete_clicked()
{
    // Messagebox delete cuestion -----------------
    QMessageBox msgBox;
    msgBox.setText(tr("Do you want to delete this topic?"));
    msgBox.setWindowTitle(tr("Delete"));
    msgBox.setIcon(QMessageBox::Warning);

    QPushButton *deleteButton = msgBox.addButton(tr("Delete"), QMessageBox::ActionRole);
    QPushButton *cancelButton = msgBox.addButton(tr("Cancel"), QMessageBox::ActionRole);
//    QPushButton *deleteButton = msgBox.addButton(QMessageBox::Ok);
//    QPushButton *cancelButton = msgBox.addButton(QMessageBox::Cancel);
    msgBox.setDefaultButton(QMessageBox::Cancel);
    msgBox.exec();
    // -----------------------------------------------

    if (msgBox.clickedButton() == deleteButton) {

        QSqlDatabase db = Database::instance().getConnection(tpc);
        QSqlQuery qry(db);

        qry.prepare("DELETE FROM topics WHERE list = ?");
        qry.addBindValue(tpc);
        if (!qry.exec()) qDebug() << qry.lastError().text();

        for (int i=0; i<8; ++i) {
            col = "T"+QString::number(i);
            qry.prepare("DELETE FROM '"+col+"' WHERE list = ?");
            qry.addBindValue(tpc);
            if (!qry.exec()) qDebug() << qry.lastError().text();
        }
        qry.finish();


    } else if (msgBox.clickedButton() == cancelButton) {
        return;
    }
}

void item_list::on_pushButton_tabmanage_share_clicked()
{
    mShare = new Share(this);

    //mShare->load_array(trgt, "learnt");

    mShare->show();
}


void SwitchToTabContaining(QTabWidget * tabsW, QWidget * w)
{
    for(int i=0;i<tabsW->count();++i) {
        QWidget * tab = tabsW->widget(i);
        if(tab->isAncestorOf(w)) {
            tabsW->setCurrentWidget(tab);
            break;
        }
    }
}


void item_list::on_pushButton_tabmanage_edit_clicked()
{
    mEditTpc = new EditTpc(this);

    //mShare->load_array(trgt, "learnt");

    mEditTpc->show();

}


void item_list::on_pushButton_tabmanage_markas_clicked()
{
    QString btext=ui->pushButton_tabmanage_markas->text();

    if (btext == "Mark as learnt") {
            ui->pushButton_tabmanage_markas->setText("Mark to learn");
    }
    else {
        ui->pushButton_tabmanage_markas->setText("Mark as learnt");
    }

}


void item_list::on_pushButton_tabmanage_trans_clicked()
{

    mTranslate = new Translate(this);

    //mShare->load_array(trgt, "learnt");

    mTranslate->show();

}


void item_list::on_pushButton_practice_clicked()
{
    this->hide();
    Practice dlg;
    dlg.setModal(true);
    dlg.exec();
}
