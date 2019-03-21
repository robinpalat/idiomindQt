#include <QDirIterator>
#include <qdebug.h>
#include <QCloseEvent>

#include "dlg_add.h"
#include "build/ui/ui_dlg_add.h"
#include "vars_statics.h"
#include "vars_session.h"

Add::Add(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Add) {
    ui->setupUi(this);
    ui->pushButton_add->setIcon(QIcon(ivar::DS+"/images/add.png"));
    ui->pushButton_more->setIcon(QIcon(ivar::DS+"/images/add_more.png"));
    ui->pushButton_list->setIcon(QIcon(ivar::DS+"/images/add_list.png"));
    ui->pushButton_audio->setIcon(QIcon(ivar::DS+"/images/add_audio.png"));
    ui->pushButton_image->setIcon(QIcon(ivar::DS+"/images/add_image.png"));
    ui->pushButton_clip->setIcon(QIcon(ivar::DS+"/images/add_clipboard.png"));
    ui->pushButton_srce->setIcon(QIcon(ivar::DS+"/images/add_translation.png"));
    ui->lineEdit_srce->hide(); field_srce = false;
    setWindowFlag(Qt::WindowStaysOnTopHint);

    load_data();
}

Add::~Add() {
    delete ui;
}

void Add::load_data() {
    QDirIterator it(DM_tl, QDir::Dirs);
    while (it.hasNext()) {
        QFileInfo substring = it.next();
        QString tpc = substring.baseName();
        if (tpc!=""){
            ui->comboBox->addItem(tpc);
        }
    }
}

void Add::on_pushButton_add_clicked() {

}

void Add::on_pushButton_more_clicked() {

}

void Add::on_pushButton_audio_clicked() {

}

void Add::on_pushButton_image_clicked() {

}

void Add::on_pushButton_clip_clicked() {

}

void Add::on_pushButton_srce_clicked()
{
    if(field_srce == false){
        ui->lineEdit_srce->show();
        field_srce = true;
     }
    else {
        ui->lineEdit_srce->hide();
        field_srce = false;
    }

}


void Add::closeEvent(QCloseEvent * event)
{
    if(this->isVisible()){
        event->ignore();
        this->hide();

    }
}
