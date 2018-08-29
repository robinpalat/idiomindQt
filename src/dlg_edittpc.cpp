#include "src/dlg_edittpc.h"
#include "ui_dlg_edittpc.h"
#include <QMenu>

EditTpc::EditTpc(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::EditTpc)
{
    ui->setupUi(this);

    load_data();

    ui->tableWidget_edit->verticalHeader()->setFixedWidth(30);
    ui->tableWidget_edit->verticalHeader()->setSectionsMovable(true);
    ui->tableWidget_edit->setAcceptDrops(true);
    ui->tableWidget_edit->setDragDropOverwriteMode(false);
    ui->tableWidget_edit->setDropIndicatorShown(true);
    ui->tableWidget_edit->verticalHeader()->setCursor(Qt::ClosedHandCursor);
    ui->tableWidget_edit->verticalHeader()->setDropIndicatorShown(true);
    ui->tableWidget_edit->verticalHeader()->setToolTip(tr("Move by drag and drop"));
    //ui->tableWidget_edit->verticalHeader()->setStyleSheet("background-color: black;");
    ui->tableWidget_edit->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    ui->tableWidget_edit->verticalHeader()->setCursor(Qt::ClosedHandCursor);
    ui->tableWidget_edit->verticalHeader()->showDropIndicator();


    ui->tableWidget_edit->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->tableWidget_edit, SIGNAL(customContextMenuRequested(QPoint)),
              SLOT(customMenuRequested(QPoint)));

}


void EditTpc::customMenuRequested(QPoint pos) {

  QMenu* menu = new QMenu(this);
  const QIcon delIcon = QIcon::fromTheme ("delete-record", QIcon(QIcon::fromTheme("edit-delete")));
  QAction* delAction = new QAction(delIcon, tr("&Delete item"), this);
  delAction->setShortcuts (QKeySequence::Delete);
  connect (delAction, &QAction::triggered, this, &EditTpc::delete_item);
  menu->addAction(delAction);
  menu->popup(ui->tableWidget_edit->viewport()->mapToGlobal(pos));
}


void EditTpc::delete_item() {

    QString trgt = ui->tableWidget_edit->item(ui->tableWidget_edit->currentRow(), 0)->text();

    QSqlDatabase mydb;
    mydb.setDatabaseName(DM_tl+"/"+tpc+"/.conf/tpcdb");
    mydb=QSqlDatabase::addDatabase("QSQLITE");
    mydb.setDatabaseName(DM_tl+"/"+tpc+"/.conf/tpcdb");
    if (!mydb.open()) qDebug()<<("Failed to open the database");
    QSqlQuery qry;

    qry.prepare("DELETE FROM "+Source_LANG+" WHERE trgt = ?");
    qry.addBindValue(trgt);
    if (!qry.exec()) qDebug() << qry.lastError().text();
    qry.prepare("DELETE FROM learning WHERE list = ?");
    qry.addBindValue(trgt);
    if (!qry.exec()) qDebug() << qry.lastError().text();
    qry.prepare("DELETE FROM learnt WHERE list = ?");
    qry.addBindValue(trgt);
    if (!qry.exec()) qDebug() << qry.lastError().text();
    qry.prepare("DELETE FROM sentences WHERE list = ?");
    qry.addBindValue(trgt);
    if (!qry.exec()) qDebug() << qry.lastError().text();
    qry.prepare("DELETE FROM marks WHERE list = ?");
    qry.addBindValue(trgt);
    if (!qry.exec()) qDebug() << qry.lastError().text();
    qry.prepare("DELETE FROM words  WHERE list = ?");
    qry.addBindValue(trgt);
    if (!qry.exec()) qDebug() << qry.lastError().text();
    qry.finish();

    mydb.close();
    mydb = QSqlDatabase();
    mydb.removeDatabase(QSqlDatabase::defaultConnection);

    load_data();
}
EditTpc::~EditTpc()
{
    delete ui;
}


QString EditTpc::get_tpc() {
    Global mGlobal;
    return  mGlobal.get_textline(ivar::FILE_mn);
}


// laod list of items
void EditTpc::load_data() {

    tpc = get_tpc();
    this->setWindowTitle("Idiomind - "+tpc);

    QSqlDatabase mydb;
    mydb.setDatabaseName(DM_tl+"/"+tpc+"/.conf/tpcdb");
    mydb=QSqlDatabase::addDatabase("QSQLITE");
    mydb.setDatabaseName(DM_tl+"/"+tpc+"/.conf/tpcdb");
    if (!mydb.open()) qDebug()<<("Failed to open the database");
    QSqlQuery* qry_a=new QSqlQuery(mydb);
    qry_a->prepare("select trgt from "+Source_LANG+"");

    ui->tableWidget_edit->setColumnCount(1);
    ui->tableWidget_edit->setRowCount(0);

    edittpc_check_items.clear();

    if (qry_a->exec( )) {
        while(qry_a->next()) {

            QString trgt = qry_a->value(0).toString();
            edittpc_load_items.push_back(trgt);

            ui->tableWidget_edit->insertRow(ui->tableWidget_edit->rowCount());
            QTableWidgetItem *itemTrgt = new QTableWidgetItem(trgt);
            ui->tableWidget_edit->setItem(ui->tableWidget_edit->rowCount() -1, 0, itemTrgt);

        }
    }

    ui->tableWidget_edit->setColumnWidth(0, 500);

    mydb.close();
    mydb = QSqlDatabase();
    mydb.removeDatabase(QSqlDatabase::defaultConnection);
}


void EditTpc::save_data() {

    this->setWindowTitle("Idiomind - "+tpc);

    QSqlDatabase mydb;
    mydb.setDatabaseName(DM_tl+"/"+tpc+"/.conf/tpcdb");
    mydb=QSqlDatabase::addDatabase("QSQLITE");
    mydb.setDatabaseName(DM_tl+"/"+tpc+"/.conf/tpcdb");
    if (!mydb.open()) qDebug()<<("Failed to open the database");

    edittpc_check_items.clear();
    QAbstractItemModel *model = ui->tableWidget_edit->model();
    //for ( int col = 0; col < model->columnCount(); ++col )
      //{
      for( int row = 0; row < model->rowCount(); ++row ) {
         QModelIndex index = model->index( row, 0 );
         QString trgt = index.data().toString();
         edittpc_check_items.push_back(trgt);

         }
     // }

   if ( edittpc_load_items != edittpc_check_items ) {
       qDebug() << "Diff...";
   }

    QString* arraytrgt = &edittpc_check_items[0];

    int n = 0;
    for(std::vector<QString>::iterator it = edittpc_load_items.begin(); it != edittpc_load_items.end(); ++it) {
        if (*it != arraytrgt[n]) {
            QSqlQuery qry;
            qry.prepare("update "+Source_LANG+" set trgt='"+arraytrgt[n]+"' where trgt='"+*it+"'");
            qry.exec();
            qry.prepare("update learning set list='"+arraytrgt[n]+"' where list='"+*it+"'");
            qry.exec();
            qry.prepare("update learnt set list='"+arraytrgt[n]+"' where list='"+*it+"'");
            qry.exec();
            qry.prepare("update sentences set list='"+arraytrgt[n]+"' where list='"+*it+"'");
            qry.exec();
            qry.prepare("update marks set list='"+arraytrgt[n]+"' where list='"+*it+"'");
            qry.exec();
            qry.prepare("update words set list='"+arraytrgt[n]+"' where list='"+*it+"'");
            qry.exec();
            qry.finish();
        }
        n++;
    }

    mydb.close();
    mydb = QSqlDatabase();
    mydb.removeDatabase(QSqlDatabase::defaultConnection);
}


void EditTpc::on_pushButton_close_clicked()
{
    this->close();
}

void EditTpc::on_pushButton_save_edits_clicked()
{
    save_data();
    ui->pushButton_save_edits->setEnabled(false);
}


