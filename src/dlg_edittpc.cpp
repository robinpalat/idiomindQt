#include "src/dlg_edittpc.h"
#include "ui_dlg_edittpc.h"

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
   // ui->tableWidget_edit->horizontalHeader()->setResizeMode(QHeaderView::Interactive);

    ui->tableWidget_edit->verticalHeader()->setCursor(Qt::ClosedHandCursor);
    ui->tableWidget_edit->verticalHeader()->showDropIndicator();
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

    QSqlDatabase mydb;

    this->setWindowTitle("Idiomind - "+tpc);

    mydb.setDatabaseName(DM_tl+"/"+tpc+"/.conf/tpcdb");

    mydb=QSqlDatabase::addDatabase("QSQLITE");
    mydb.setDatabaseName(DM_tl+"/"+tpc+"/.conf/tpcdb");

    if (!mydb.open()) qDebug()<<("Failed to open the database");


    QSqlQuery* qry_a=new QSqlQuery(mydb);
    qry_a->prepare("select trgt from "+Source_LANG+"");
    ui->tableWidget_edit->setColumnCount(1);
    ui->tableWidget_edit->setRowCount(0);

    numberOfRows = 0;
    if (qry_a->exec( )) {
        while(qry_a->next()) {
            QString trgt = qry_a->value(0).toString();
             edittpc_load_items.push_back(trgt);
              qDebug() << trgt+QString::number(numberOfRows)+"A";
            //TableWidgetItem *item2 = new QTableWidgetItem("");
            //item2->setCheckState(Qt::Unchecked);
            ui->tableWidget_edit->insertRow(ui->tableWidget_edit->rowCount());

            QTableWidgetItem *itemTrgt = new QTableWidgetItem(trgt);



            ui->tableWidget_edit->setItem(ui->tableWidget_edit->rowCount() -1, 0, itemTrgt);
            //ui->list_learning->setItem(ui->list_learning->rowCount() -1, 1, item2);


            numberOfRows ++;
        }
    }


     ui->tableWidget_edit->setColumnWidth(0, 500);


    mydb.close();
    mydb = QSqlDatabase();
    mydb.removeDatabase(QSqlDatabase::defaultConnection);


}


void EditTpc::save_data() {

    QSqlDatabase mydb;

    this->setWindowTitle("Idiomind - "+tpc);

    mydb.setDatabaseName(DM_tl+"/"+tpc+"/.conf/tpcdb");

    mydb=QSqlDatabase::addDatabase("QSQLITE");
    mydb.setDatabaseName(DM_tl+"/"+tpc+"/.conf/tpcdb");

    if (!mydb.open()) qDebug()<<("Failed to open the database");


    edittpc_check_items.clear();
    numberOfRowsx = 0;
    QAbstractItemModel *model = ui->tableWidget_edit->model();
    //for ( int col = 0; col < model->columnCount(); ++col )
      //{
      for( int row = 0; row < model->rowCount(); ++row )
         {
         QModelIndex index = model->index( row, 0 );
         QString trgt = index.data().toString();
         edittpc_load_items.push_back(trgt);
         qDebug() << trgt+QString::number(numberOfRowsx)+"B";
         numberOfRowsx++;

         }
     // }



   if ( edittpc_load_items != edittpc_check_items ) {
       qDebug() << numberOfRowsx << endl << numberOfRows;
   }

//        QString* arraytrgt = &edittpc_check_items[0];

//        int n = 0;
//        for(std::vector<QString>::iterator it = edittpc_load_items.begin(); it != edittpc_load_items.end(); ++it) {
//            if (*it != arraytrgt[n]) {
//                QSqlQuery qry;
//                qry.prepare("update "+Source_LANG+" set srce='"+arraytrgt[n]+"' where srce='"+*it+"'");
//                qry.exec();
//            }
//            n++;
//        }
//    }
//    mydb.close();
//    mydb = QSqlDatabase();
//    mydb.removeDatabase(QSqlDatabase::defaultConnection);


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


