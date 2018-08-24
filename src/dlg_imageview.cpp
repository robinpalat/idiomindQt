#include "dlg_imageview.h"
#include "ui_dlg_imageview.h"

Dlg_ImageView::Dlg_ImageView(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dlg_ImageView)
{
    ui->setupUi(this);

}


void Dlg_ImageView::load_image(QString trgt)
{
    QString userimg="/home/robin/Shortcuts/English/.share/images/"+trgt.toLower()+"-1.jpg";
    ui->Label_image_set->setPixmap(QPixmap(userimg) );

}


Dlg_ImageView::~Dlg_ImageView()
{
    delete ui;
}
