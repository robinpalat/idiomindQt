#include "dlg_vwr.h"
#include "build/ui/ui_dlg_vwr.h"
#include "vars_statics.h"

#include <QSqlQuery>
#include <QPixmap>
#include <QCloseEvent>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>

#include <iostream>
#include <sstream>
#include <vector>
#include <QTimer>
#include <QPainter>


Vwr::Vwr(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Vwr) {

    ui->setupUi(this);
    QSettings settings(ivar::FILE_conf, QSettings::IniFormat);
    restoreGeometry(settings.value("dlgviewer").toByteArray());
    dark = settings.value("darkviewer").toBool();
    white_dark();
    ui->tableWidget_wrdsList->setColumnCount(2);
    ui->tableWidget_wrdsList->setColumnWidth(0, 255);
    ui->tableWidget_wrdsList->setColumnWidth(1, 255);
    ui->label_image->setScaledContents( true );
    this->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(QPoint)),
              SLOT(customMenuRequested(QPoint)));

    player = new QMediaPlayer(this);
}

void Vwr::change_white_dark() {

    if (dark == false) dark = true;
    else dark = false;
    white_dark();
}

void Vwr::white_dark() {

    if (dark == true) {
        ui->widget->setStyleSheet("background-color: rgb(55, 55, 55);");
        ui->widget_2->setStyleSheet("background-color: rgb(55, 55, 55);");
        ui->pushButton_next->setStyleSheet("color: rgb(200, 200, 200);border-radius: 5px;border-width: 0px;background-color: #424242;");
        ui->pushButton_prev->setStyleSheet("color: rgb(200, 200, 200);border-radius: 5px;border-width: 0px;background-color: #424242;");
        ui->label_trgt->setStyleSheet("color: #EAEAEA;");
        ui->label_srce->setStyleSheet("color: #C1C1C1;");
        ui->label_note->setStyleSheet("color: #EAEAEA;");
        ui->label_exmp->setStyleSheet("color: #C1C1C1;");
        ui->label_defn->setStyleSheet("color: #EAEAEA;");
        ui->label_image->setStyleSheet("QLabel {border: 2px solid '#4A4A4A';border-radius: 6px;padding: 0px 0px 0px 0px;}");
    }
    else  {
        ui->widget->setStyleSheet("background-color: rgb(255, 255, 255);");
        ui->widget_2->setStyleSheet("background-color: rgb(255, 255, 255);");
        ui->pushButton_next->setStyleSheet("color: rgb(70, 70, 70);border-radius: 5px;border-width: 0px;background-color: #f8f8f8;");
        ui->pushButton_prev->setStyleSheet("color: rgb(70, 70, 70);border-radius: 5px;border-width: 0px;background-color: #f8f8f8;");
        ui->label_trgt->setStyleSheet("color: #2D2D2D;");
        ui->label_srce->setStyleSheet("color: #676767;");
        ui->label_note->setStyleSheet("color: #2D2D2D;");
        ui->label_exmp->setStyleSheet("color: #676767;");
        ui->label_defn->setStyleSheet("color: #2D2D2D;");
        ui->label_image->setStyleSheet("QLabel {border: 2px solid '#E6E6E6';border-radius: 6px;padding: 0px 0px 0px 0px;}");
    }
    word_list(wrds);
}

void Vwr::load_array(QString trgt_ind, QString list_sel) {

    QString t, s;
    QSqlDatabase db = Database::instance().getConnection(tpc);
    QSqlQuery qry(db);
    trgt = trgt_ind;
    list = list_sel;

    if (qry.prepare("select list from '"+list+"'") != true ) {
        qDebug() << "DB no prepared - vwr";
    };

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

//        font = self.font()
//        font.setPixelSize(self.height() * 0.8)
//        self.setFont(font)

void Vwr::word_list(QString wrds){

    std::string s = wrds.toStdString();
    std::string::size_type prev_pos = 0, pos = 0;
    bool lo = false;
    ui->tableWidget_wrdsList->setRowCount(0);
    while( (pos = s.find('_', pos)) != std::string::npos )
    {
        std::string substring( s.substr(prev_pos, pos-prev_pos) );
        if (lo==false) {
            ui->tableWidget_wrdsList->insertRow(ui->tableWidget_wrdsList->rowCount());
            QTableWidgetItem * abocada = new QTableWidgetItem( QString::fromStdString(substring));
            if (dark==true) abocada->setForeground(QBrush(QColor(215, 215, 215)));
            else abocada->setForeground(QBrush(QColor(48, 48, 48)));
            ui->tableWidget_wrdsList->setItem(ui->tableWidget_wrdsList->rowCount() -1, 0, abocada);
            lo=true;
        }
        else {
            QTableWidgetItem * abocado = new QTableWidgetItem( QString::fromStdString(substring));
            if (dark==true) abocado->setForeground(QBrush(QColor(215, 215, 215)));
            else abocado->setForeground(QBrush(QColor(48, 48, 48)));
            ui->tableWidget_wrdsList->setStyleSheet("QTableWidget::item:selected{background-color:'#FFFFFF'};");
            ui->tableWidget_wrdsList->setItem(ui->tableWidget_wrdsList->rowCount() -1, 1, abocado);
            lo=false;
        }
        prev_pos = ++pos;
    }
}


//item = QTableWidgetItem('text')
//       item.setForeground(QBrush(QColor(0, 255, 0)))




void Vwr::setLabelText(QString trgt){

    ui->label_srce->setText(" ");
    ui->label_trgt->setText(" ");
    //ui->widget_sizelimit->hide();
    //QTimer::singleShot(100, ui->widget_sizelimit, &QWidget::show);
    QSqlDatabase db = Database::instance().getConnection(tpc);
    QSqlQuery qry(db);
    qry.prepare("select * from Data where trgt=(:trgt_val)");



    qry.bindValue(":trgt_val", trgt);





    if (qry.exec( )) {
        while(qry.next()) {
            srce = qry.value(1).toString();
            grmr = qry.value(6).toString();
            wrds = qry.value(5).toString();
            type = qry.value(13).toString();
            note = qry.value(4).toString();
            defn = qry.value(3).toString();
            exmp = qry.value(2).toString();
        }
    }




    if (type == "2") {
        word_list(wrds);
    }
    if (type == "1") {

        QString userimg;

        QString userimg0=DM_tl+"/.share/images/"+trgt.toLower()+"-0.jpg";
        QString userimg1=DM_tl+"/.share/images/"+trgt.toLower()+"-1.jpg";
        QString userimg2=DM_tl+"/"+tpc+"/images/"+trgt.toLower()+".jpg";

        if(QFileInfo(userimg0).exists()) {
            userimg = userimg0;
        }
        else if (QFileInfo(userimg1).exists()) {
            userimg = userimg1;
        }
        else if (QFileInfo(userimg2).exists()) {
                userimg = userimg2;
        }

        if(QFileInfo(userimg).exists()){
            ui->label_image->show();
            ui->label_image->setPixmap(QPixmap(userimg) );
            ui->label_trgt->setAlignment(Qt::AlignLeft);
            ui->label_srce->setAlignment(Qt::AlignLeft);
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
        ui->label_trgt->setText(trgt);
        ui->label_srce->setText(srce);
        ui->label_exmp->setText(exmp);
        ui->label_defn->setText(defn);
        ui->label_note->setText(note);
        QGraphicsOpacityEffect *effect = new QGraphicsOpacityEffect();
        ui->widget_2->setGraphicsEffect(effect);
        QPropertyAnimation *anim = new QPropertyAnimation(effect,"opacity");
        anim->setDuration(300);
        anim->setStartValue(0.0);
        anim->setEndValue(1.0);
        anim->setEasingCurve(QEasingCurve::OutQuad);
        connect(anim, &QPropertyAnimation::finished, [=]()
        {
            ui->widget_2->show();
        });

        anim->start(QAbstractAnimation::KeepWhenStopped);

        //QTimer::singleShot(1000, [&](){ ui->label_srce->setText(srce);});

    }
    else if (type == "2") {
        ui->label_image->hide();
        QFont font_trgt = ui->label_trgt->font();
        font_trgt.setPointSize(16);
        ui->label_trgt->setAlignment(Qt::AlignLeft);
        ui->label_trgt->setFont(font_trgt);
        QFont font_srce = ui->label_srce->font();
        font_srce.setPointSize(10);
        ui->label_srce->setAlignment(Qt::AlignLeft);
        //font_srce.setStyle()
        ui->label_srce->setFont(font_srce);
        ui->label_trgt->setContentsMargins(0,0,0,0);
        ui->label_srce->setContentsMargins(0,0,0,0);
        ui->tableWidget_wrdsList->show();
        ui->label_exmp->hide();
        ui->label_defn->hide();
        ui->label_trgt->setText(grmr);
        ui->label_srce->setText(srce);
        QGraphicsOpacityEffect *effect = new QGraphicsOpacityEffect();
        ui->widget_2->setGraphicsEffect(effect);
        QPropertyAnimation *anim = new QPropertyAnimation(effect,"opacity");
        anim->setDuration(300);
        anim->setStartValue(0.0);
        anim->setEndValue(1.0);
        anim->setEasingCurve(QEasingCurve::OutQuad);
        connect(anim, &QPropertyAnimation::finished, [=]()
        {
            ui->widget_2->show();
        });
        anim->start(QAbstractAnimation::KeepWhenStopped);
        // QTimer::singleShot(1000, [&](){ ui->label_srce->setText(srce);});
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


void Vwr::closeEvent( QCloseEvent* event ) {

    QSettings settings(ivar::FILE_conf, QSettings::IniFormat);
    settings.setValue("dlgviewer", saveGeometry());
    settings.setValue("darkviewer", dark);
}

//void Vwr::on_label_srce_clicked() {
//    on_label_trgt_clicked();
//}

void Vwr::on_label_trgt_clicked() {

    Audioplayer path;
    player->setSource(QUrl::fromLocalFile(path.pathplay(trgt)));
    player->play();
}


void Vwr::on_edit() {

        this->close();
        mDlg_editItem = new Dlg_editItem(this);
        mDlg_editItem->load_data(trgt, list);
        mDlg_editItem->show();
}

void Vwr::customMenuRequested(QPoint pos) {

  QMenu* menu = new QMenu(this);

  const QIcon delIcon = QIcon::fromTheme ("delete-record", QIcon(QIcon::fromTheme("edit-delete")));
  QAction* delAction = new QAction(delIcon, tr("&Mark item"), this);
  delAction->setShortcuts (QKeySequence::Delete);
  connect (delAction, &QAction::triggered, this, &Vwr::on_edit);
  menu->addAction(delAction);
  const QIcon editIcon = QIcon::fromTheme ("delete-record", QIcon(ivar::DS+"/images/add_more.png"));
  QAction* editaction = new QAction(editIcon, tr("&Edit"), this);
  connect (editaction, &QAction::triggered, this, &Vwr::on_edit);
  menu->addAction(editaction);
  const QIcon darkIcon = QIcon::fromTheme ("delete-record", QIcon(ivar::DS+"/images/add_more.png"));
  QAction* darkaction = new QAction(darkIcon, tr("&Dark"), this);
  connect (darkaction, &QAction::triggered, this, &Vwr::change_white_dark);
  menu->addAction(darkaction);
  menu->popup(this->mapToGlobal(pos));
}
