#include "dlg_topics.h"
#include "build/ui/ui_dlg_topics.h"
#include "vars_statics.h"
#include <qdebug.h>
#include "vars_session.h"
#include "icontray.h"

#include <QCloseEvent>
#include <QDirIterator>

Topics::Topics(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Topics)
{
    ui->setupUi(this);

    mGlobal = new Global();

    QString tpc = mGlobal->get_textline(ivar::FILE_mn);
    load_index();

}

Topics::~Topics()
{
    delete ui;
}


void Topics::on_pushButton_clicked()
{
 this->hide();
}

void Topics::load_index()
{
    ui->tableWidget_topics->setColumnCount(2);
    ui->tableWidget_topics->setRowCount(0);
    ui->tableWidget_topics->setColumnWidth(0, 42);
    ui->tableWidget_topics->setColumnWidth(1, 485);
    ui->tableWidget_topics->setStyleSheet("QTableWidget::item { padding: 1px }");
    ui->tableWidget_topics->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    QDirIterator it(DM_tl, QDir::Dirs);
    while (it.hasNext()) {
        QFileInfo substring = it.next();
        QString tpc = substring.baseName();
        if (tpc!=""){

            QString Home = QDir::homePath();
            QString FILE_mn = DM_tl+"/"+tpc+"/.conf/stts";
            Global mGlobal;
            QString stts = mGlobal.get_textline(FILE_mn);

            QString imgPath = ivar::DS+"/images/img."+stts+".png";
            QImage *img = new QImage(imgPath);

            QTableWidgetItem *thumbnail = new QTableWidgetItem;
            thumbnail->setData(Qt::DecorationRole, QPixmap::fromImage(*img));

            ui->tableWidget_topics->insertRow(ui->tableWidget_topics->rowCount());
            ui->tableWidget_topics->setItem(ui->tableWidget_topics->rowCount() -1, 0, thumbnail);
            ui->tableWidget_topics->setItem(ui->tableWidget_topics->rowCount() -1, 1, new QTableWidgetItem(tpc));
        }
    }
}


void Topics::closeEvent(QCloseEvent * event)
{
    if(this->isVisible()){
        event->ignore();
        this->hide();
    }
}


void Topics::on_tableWidget_topics_cellDoubleClicked()
{
    QModelIndex currentIndex = ui->tableWidget_topics->currentIndex();
    QString tpc = ui->tableWidget_topics->item(currentIndex.row(), 1)->text();
    qDebug() << "\""+tpc+ "\" is your topic now";

    QString filename=ivar::FILE_mn;
    QFile file( filename );
    if ( file.open(QIODevice::ReadWrite | QIODevice::Truncate | QIODevice::Text))
    {
        QTextStream stream( &file );
        stream << tpc;
    }
    this->hide();

//    mMainWindow = new MainWindow(this);
//    mMainWindow->load_data();
//    mMainWindow->show();
}

void Topics::on_pushButton_2_clicked()
{
    QModelIndex currentIndex = ui->tableWidget_topics->currentIndex();
    QString tpc = ui->tableWidget_topics->item(currentIndex.row(), 1)->text();
    qDebug() << "\""+tpc+ "\" is your topic now";

    QString filename=ivar::FILE_mn;
    QFile file( filename );
    if ( file.open(QIODevice::ReadWrite | QIODevice::Truncate | QIODevice::Text))
    {
        QTextStream stream( &file );
        stream << tpc;
    }
    this->hide();
}
