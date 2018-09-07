#include "dlg_vwr.h"
#include "build/ui/ui_dlg_vwr.h"
#include "vars_statics.h"

#include <QSqlQuery>
#include <QPixmap>
#include <QCloseEvent>

#include <iostream>
#include <sstream>
#include <vector>

using namespace std;

Vwr::Vwr(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Vwr)
{
    ui->setupUi(this);

    ui->tableWidget_wrdsList->setColumnCount(2);
    ui->tableWidget_wrdsList->setColumnWidth(0, 255);
    ui->tableWidget_wrdsList->setColumnWidth(1, 255);

    ui->label_image->setScaledContents( true );

    //ui->pushButton_2->setIcon(QIcon(ivar::DS+"/images/listen.png"));
    player = new QMediaPlayer(this);
}

void Vwr::load_array(QString trgt_ind, QString list_sel)
{
    QString t, s;

    QSqlDatabase db = Database::instance().getConnection(DM_tl+"/"+tpc+"/.conf/tpcdb");
    QSqlQuery qry(db);

    trgt = trgt_ind;
    list = list_sel;
    qry.prepare("select list from '"+list+"'");

   if (qry.exec()) {

        loaded = true;
        unsigned long int indexpos = 0;
        while(qry.next()) {
            t = qry.value(0).toString();
            arr.push_back(t);
            items++;

            if (t == trgt_ind) {
                pos = indexpos;
                setLabelText(trgt);
            }
            else {
                indexpos++;
            }
        }
   items = items-1;
   }
   else {
     QMessageBox::critical(this, tr("Error"), qry.lastError().text());
   }

    qry.finish();
}

void Vwr::setLabelText(QString trgt)
{
    QSqlDatabase db = Database::instance().getConnection(DM_tl+"/"+tpc+"/.conf/tpcdb");
    QSqlQuery qry(db);

    qry.prepare("select * from "+Source_LANG+" where trgt=(:trgt_val)");
    qry.bindValue(":trgt_val", trgt);

    QString grmr, wrds, type;
    if (qry.exec( )) {
        while(qry.next()) {
            srce = qry.value(1).toString();
            grmr = qry.value(8).toString();
            wrds = qry.value(9).toString();
            type = qry.value(13).toString();

            note = qry.value(4).toString();
            defn = qry.value(3).toString();
            exmp = qry.value(2).toString();
        }
    }
    if (type == "2") {
        std::string s = wrds.toStdString();
        std::string::size_type prev_pos = 0, pos = 0;
        bool lo = false;
        ui->tableWidget_wrdsList->setRowCount(0);
        while( (pos = s.find('_', pos)) != std::string::npos )
        {
            std::string substring( s.substr(prev_pos, pos-prev_pos) );
            if (lo==false) {
                ui->tableWidget_wrdsList->insertRow(ui->tableWidget_wrdsList->rowCount());
                ui->tableWidget_wrdsList->setItem(ui->tableWidget_wrdsList->rowCount() -1, 0,
                                                  new QTableWidgetItem( QString::fromStdString(substring)));
                lo=true;
            }
            else {
                QTableWidgetItem * abocado = new QTableWidgetItem( QString::fromStdString(substring));
                abocado->setTextColor("#5F5F5F");

                //ui->tableWidget_wrdsList->setStyleSheet("QTableWidget::item:selected{background-color:'#FFFFFF'};");

                ui->tableWidget_wrdsList->setItem(ui->tableWidget_wrdsList->rowCount() -1, 1, abocado);
                lo=false;
            }
            prev_pos = ++pos;
        }
    }

    if (type == "1") {

        QString userimg=DM_tl+"/.share/images/"+trgt.toLower()+"-1.jpg";
        //ui->label_image->setStyleSheet("QLabel {border: 1px solid '#BDBDBD';border-radius: 4px;padding: 0px 0px 0px 0px;}");
        //ui->label_image->setMargin(3);

        if(QFileInfo(userimg).exists()){
            ui->label_image->show();
            ui->label_image->setPixmap(QPixmap(userimg) );
            ui->label_trgt->setAlignment(Qt::AlignCenter);
            ui->label_srce->setAlignment(Qt::AlignCenter);
            ui->label_trgt->setContentsMargins(25,0,0,0);
            ui->label_srce->setContentsMargins(28,0,0,0);
        }
        else{
            ui->label_image->hide();
            ui->label_trgt->setAlignment(Qt::AlignCenter);
            ui->label_srce->setAlignment(Qt::AlignCenter);
            ui->label_trgt->setContentsMargins(0,0,0,0);
            ui->label_srce->setContentsMargins(0,0,0,0);
        }

        QFont font_trgt = ui->label_trgt->font();
        font_trgt.setPointSize(28);

        ui->label_trgt->setFont(font_trgt);

        QFont font_srce = ui->label_srce->font();
        font_srce.setPointSize(16);

        ui->label_srce->setFont(font_srce);

        ui->tableWidget_wrdsList->hide();
        ui->label_exmp->show();
        ui->label_defn->show();

        ui->label_trgt->setText(trgt); // label dialog
        ui->label_srce->setText("<font color='#494949'>"+srce+"</font>"); // label dialog

        ui->label_exmp->setText(exmp);
        ui->label_defn->setText(defn);
        ui->label_note->setText(note);

    }
    else if (type == "2") {

        ui->label_image->hide();

        QFont font_trgt = ui->label_trgt->font();
        font_trgt.setPointSize(18);
        ui->label_trgt->setAlignment(Qt::AlignLeft);
        ui->label_trgt->setFont(font_trgt);

        QFont font_srce = ui->label_srce->font();
        font_srce.setPointSize(12);
        ui->label_srce->setAlignment(Qt::AlignLeft);
        //font_srce.setStyle()
        ui->label_srce->setFont(font_srce);

        ui->label_trgt->setContentsMargins(0,0,0,0);
        ui->label_srce->setContentsMargins(0,0,0,0);

        ui->tableWidget_wrdsList->show();
        ui->label_exmp->hide();
        ui->label_defn->hide();

        ui->label_trgt->setText(grmr); // label dialog
        ui->label_srce->setText("<font color='#494949'>"+srce+"</font>"); // label dialog
    }

    qry.finish();
}

Vwr::~Vwr() {

    delete ui;
}

void Vwr::on_pushButton_next_clicked() {
    pos++;
    if (pos>items) pos = 0;
    trgt = arr[pos];

    setLabelText(trgt);

}

void Vwr::on_pushButton_prev_clicked() {

    checkpos = pos;
    checkpos--;
    if (checkpos<0) {pos = items;}
    else {pos--;}
    trgt = arr[pos];

    setLabelText(trgt);
}

void Vwr::on_label_image_clicked() {

    //setConnect_lABEL_IMAGE();
    mDlg_ImageView = new Dlg_ImageView(this);
    mDlg_ImageView->load_image(trgt);
    mDlg_ImageView->show();
}



void Vwr::on_label_note_clicked() {

    //setConnect_lABEL_IMAGE();
    ui->label_note->hide();
    //ui->pushButton_noteSave->show();

}


//void Vwr::on_pushButton_edit_clicked()
//{
//    Vwr conn;
//    conn.connClose();
//    this->close();
//    mDlg_editItem = new Dlg_editItem(this);
//    mDlg_editItem->load_data(trgt, list);

//    mDlg_editItem->show();
//}


//void Vwr::closeEvent( QCloseEvent* event )
//{
//    Vwr conn;
//    emit conn.connClose();
//    event->accept();
//}

void Vwr::on_label_srce_clicked() {

    on_label_trgt_clicked();
}

void Vwr::on_label_trgt_clicked() {

    Audioplayer path;
    player->setMedia(QUrl::fromLocalFile(path.pathplay(trgt)));
    player->play();
}


void Vwr::on_label_trgt_doubleClicked() {

        this->close();
        mDlg_editItem = new Dlg_editItem(this);
        mDlg_editItem->load_data(trgt, list);

        mDlg_editItem->show();
}
