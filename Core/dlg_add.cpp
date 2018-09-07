#include <QDirIterator>
#include <qdebug.h>
#include <QCloseEvent>

#include "dlg_add.h"
#include "build/ui/ui_dlg_add.h"
#include "vars_statics.h"
#include "vars_session.h"

Add::Add(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Add)
{
    ui->setupUi(this);

    ui->pushButton_add->setIcon(QIcon::fromTheme("list-add"));
    ui->pushButton_more->setIcon(QIcon::fromTheme("document-edit"));
    ui->pushButton_audio->setIcon(QIcon::fromTheme("audio-x-generic"));
    ui->pushButton_image->setIcon(QIcon::fromTheme("insert-image"));
    ui->pushButton_clip->setIcon(QIcon::fromTheme("edit-paste"));
    ui->pushButton_srce->setIcon(QIcon::fromTheme("config-language"));
    ui->lineEdit_srce->hide(); field_srce = false;

    load_data();
}

Add::~Add()
{
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

void Add::on_pushButton_add_clicked()
{

}

void Add::on_pushButton_more_clicked()
{

}

void Add::on_pushButton_audio_clicked()
{

}

void Add::on_pushButton_image_clicked()
{

}

void Add::on_pushButton_clip_clicked()
{

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
