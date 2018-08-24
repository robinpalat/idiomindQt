#include "src/dlg_edittpc.h"
#include "ui_dlg_edittpc.h"

EditTpc::EditTpc(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::EditTpc)
{
    ui->setupUi(this);

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

    QSqlDatabase mydb;

    this->setWindowTitle("Idiomind - "+tpc);

    mydb.setDatabaseName(ivar::DM_tl+"/"+tpc+"/.conf/tpcdb");

    mydb=QSqlDatabase::addDatabase("QSQLITE");
    mydb.setDatabaseName(ivar::DM_tl+"/"+tpc+"/.conf/tpcdb");

    if (!mydb.open()) qDebug()<<("Failed to open the database");


    QSqlQuery* qry_a=new QSqlQuery(mydb);
    qry_a->prepare("select trgt from Data");
    ui->tableWidget_edit->setColumnCount(1);
    ui->tableWidget_edit->setRowCount(0);

    int numberOfRows = 0;
    if (qry_a->exec( )) {
        while(qry_a->next()) {
            QString trgt = qry_a->value(0).toString();
            //TableWidgetItem *item2 = new QTableWidgetItem("");
            //item2->setCheckState(Qt::Unchecked);
            ui->tableWidget_edit->insertRow(ui->tableWidget_edit->rowCount());
            ui->tableWidget_edit->setItem(ui->tableWidget_edit->rowCount() -1, 0, new QTableWidgetItem(trgt));
            //ui->list_learning->setItem(ui->list_learning->rowCount() -1, 1, item2);
            numberOfRows ++;
        }
    }


     ui->tableWidget_edit->setColumnWidth(0, 500);

    mydb.close();
    mydb = QSqlDatabase();
    mydb.removeDatabase(QSqlDatabase::defaultConnection);


}





void EditTpc::on_pushButton_cancel_clicked()
{
    this->close();
}

void EditTpc::on_pushButton_save_clicked()
{

}

void EditTpc::on_pushButton_more_clicked()
{

}

void EditTpc::on_pushButton_close_clicked()
{
    this->close();
}

void EditTpc::on_pushButton_save_edits_clicked()
{
    ui->pushButton_save_edits->setEnabled(false);
}
