#include "dlg_share.h"
#include "ui_dlg_share.h"
#include "session.h"
#include "global.h"

Share::Share(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Share)
{
    ui->setupUi(this);
    QString lang = ivar::tlng;
    ui->label_link_library->setText("<a href=\"http://idiomind.net/"+lang.toLower()+"\">Visit the topics library</a>");
    ui->label_link_library->setTextFormat(Qt::RichText);
    ui->label_link_library->setTextInteractionFlags(Qt::TextBrowserInteraction);
    ui->label_link_library->setOpenExternalLinks(true);
    load_data();

}

Share::~Share()
{
    delete ui;
}

QString Share::get_tpc() {
    Global mGlobal;
    return  mGlobal.get_textline(ivar::FILE_mn);
}

void Share::load_data() {

    tpc = get_tpc();

    // db
//    QSqlDatabase mydb;
//    mydb.setDatabaseName(ivar::DM_tl+"/"+tpc+"/.conf/tpcdb");
//    mydb=QSqlDatabase::addDatabase("QSQLITE");
//    mydb.setDatabaseName(ivar::DM_tl+"/"+tpc+"/.conf/tpcdb");
//    if (!mydb.open()) qDebug()<<("Failed to open the database");

//    QSqlQueryModel * tab2_modal=new QSqlQueryModel();
//    QSqlQuery* qry_b=new QSqlQuery(mydb);
//    qry_b->prepare("select list from learnt");
//    qry_b->exec();
//    tab2_modal->setQuery(*qry_b);

//    mydb.close();
//    mydb = QSqlDatabase();
//    mydb.removeDatabase(QSqlDatabase::defaultConnection);

    // load note
    QFile file(ivar::DM_tl+"/"+tpc+"/.conf/note");
    if(!file.open(QIODevice::ReadOnly)) {
        QMessageBox::information(nullptr, "error", file.errorString());
    }
    QTextStream in(&file);
    QString text = in.readAll();
    ui->textEdit_notes->setHtml(text);
}




void Share::on_pushButton_close_clicked()
{
    this->close();
}

void Share::on_pushButton_upload_clicked()
{

}

void Share::on_pushButton_export_clicked()
{

}
