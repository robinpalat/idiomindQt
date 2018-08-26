#include "dlg_edititem.h"
#include "ui_dlg_edititem.h"
#include "dlg_vwr.h"

#include <QMessageBox>

#include <iostream>
#include <sstream>
#include <vector>
#include <string>

#include <QDebug>

using namespace std;


Dlg_editItem::Dlg_editItem(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dlg_editItem)
{
    ui->setupUi(this);
}


Dlg_editItem::~Dlg_editItem()
{
    delete ui;
}


void Dlg_editItem::load_data(QString trgt_ind, QString list_sel)
{
    trgt = trgt_ind;
    list = list_sel;
    connOpenb();
    QString t, s;
    QSqlQuery qry;
    qry.prepare("select list from '"+list+"'");

   if (qry.exec()) {

        unsigned long int indexpos = 0;
        while(qry.next()) {
            t = qry.value(0).toString();
            arr.push_back(t);
            items++;

            if (t == trgt) {
                pos = indexpos;
                fill_data(trgt);
            }
            else {
                indexpos++;
            }
        }
        items = items-1;
   }
   else {
     QMessageBox::critical(this, tr("Error"), qry.lastError().text());
     connCloseb();
   }
    qry.finish();
}


void Dlg_editItem::fill_data(QString trgt)
{
    ui->lineEdit_trgt->setText("");
    ui->lineEdit_srce->setText("");
    ui->plainTextEdit_exmp->clear();
    ui->plainTextEdit_defn->clear();
    ui->plainTextEdit_note->clear();
    ui->checkBox_editItem_mark->setChecked(false);

    QSqlQuery qry;
    QString trgt_qry = trgt.replace("'", "''");
    qry.prepare("select * from Data where trgt='"+trgt_qry+"'");

    if (qry.exec( )) {
        while(qry.next()) {
            srce = qry.value(1).toString();
            grmr = qry.value(8).toString();
            wrds = qry.value(9).toString();
            type = qry.value(13).toString();

            note = qry.value(4).toString();
            defn = qry.value(3).toString();
            exmp = qry.value(2).toString();
            mark = qry.value(11).toString();
        }
    }

    if (type == "1") {

        ui->lineEdit_trgt->setText(trgt);
        ui->lineEdit_srce->setText(srce);

        ui->plainTextEdit_exmp->insertPlainText(exmp);
        ui->plainTextEdit_defn->insertPlainText(defn);
        ui->plainTextEdit_note->insertPlainText(note);
    }
    else if (type == "2") {

        ui->lineEdit_trgt->setText(trgt);
        ui->lineEdit_srce->setText(srce);

        ui->plainTextEdit_note->insertPlainText(note);
    }

    if (mark=="TRUE") ui->checkBox_editItem_mark->setChecked(true);

    qry.finish();
}


void Dlg_editItem::save_data() {


    QString trgt_mod, srce_mod, note_mod,
            defn_mod, exmp_mod, mark_mod;

    trgt_mod=ui->lineEdit_trgt->text();
    srce_mod=ui->lineEdit_srce->text();

    note_mod=ui->plainTextEdit_note->toPlainText();
    defn_mod=ui->plainTextEdit_defn->toPlainText();
    exmp_mod=ui->plainTextEdit_exmp->toPlainText();

    if(ui->checkBox_editItem_mark->checkState() == Qt::Unchecked) mark_mod = "";
    else mark_mod = "TRUE";

    if (trgt_mod!=trgt) {
        QSqlQuery qry;
        qry.prepare("update Data set trgt='"+trgt_mod+"' where trgt='"+trgt+"'");
        qry.exec();
        QSqlQuery qry2;
        qry2.prepare("update '"+list+"' set list='"+trgt_mod+"' where list='"+trgt+"'");
        qry2.exec();

        if (type == "1") {
            QSqlQuery qry;
            qry.prepare("update words set list='"+trgt_mod+"' where list='"+trgt+"'");
            qry.exec();
        }
        else if (type == "2") {
            QSqlQuery qry;
            qry.prepare("update sentences set list='"+trgt_mod+"' where list='"+trgt+"'");
            qry.exec();
        }
    }

    if (srce_mod!=srce) {
        QSqlQuery qry;
        qry.prepare("update Data set srce='"+srce_mod+"' where trgt='"+trgt+"'");
        qry.exec();
    }

    if (exmp_mod != exmp || defn_mod != defn ||  note_mod != note) {
        QSqlQuery qry;
        qry.prepare("update Data set exmp='"+exmp_mod+"', defn='"+defn_mod+"', note='"+note_mod+"' where trgt='"+trgt+"'");
        qry.exec();
    }

    if (mark_mod!=mark) {
        QSqlQuery qry;
        qry.prepare("update Data set mark='"+mark_mod+"' where trgt='"+trgt+"'");
        qry.exec();
    }

    trgt = trgt_mod;
    load_data(trgt, list);
}

void Dlg_editItem::on_pushButton_editItem_next_clicked()
{
    save_data();

    pos++;
    if (pos>items) pos = 0;
    trgt = arr[pos];

    fill_data(trgt);

}


void Dlg_editItem::on_pushButton_editItem_prev_clicked()
{
    save_data();

    checkpos = pos;
    checkpos--;
    if (checkpos<0) {pos = items;}
    else {pos--;}
    trgt = arr[pos];

    fill_data(trgt);
}


void Dlg_editItem::on_pushButton_editItem_close_clicked()
{
    this->close();
}


void Dlg_editItem::closeEvent( QCloseEvent* event )
{
    save_data();

    connCloseb();
    this->close();
    Vwr * mVwr;
    mVwr = new Vwr(this);
    mVwr->load_array(trgt, list);
    if (mod==true) accept();
    else  reject();
    mVwr->show();
}
