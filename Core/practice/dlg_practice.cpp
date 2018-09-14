#include <QCloseEvent>

#include "dlg_practice.h"
#include "build/ui/ui_dlg_practice.h"
#include "Core/vars_statics.h"
#include "Core/vars_session.h"


Practice::Practice(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Practice)
{
    ui->setupUi(this);

    ui->widget_final->hide();
    ui->widget_score->hide();
    ui->tableWidget->setColumnCount(2);
    ui->tableWidget->setRowCount(0);
    ui->tableWidget->setColumnWidth(0, 68);
    ui->tableWidget->setColumnWidth(1, 360);
    QHeaderView *verticalHeader = ui->tableWidget->verticalHeader();
    verticalHeader->setSectionResizeMode(QHeaderView::Fixed);
    verticalHeader->setDefaultSectionSize(68);
    //ui->tableWidget->setStyleSheet("QTableWidget::item { margin-right: 20px }");

    ui->verticalLayout_2->setSizeConstraint(QLayout::SetFixedSize);


    get_icons_stats();
    show_icons_stats();
}


Practice::~Practice() {

    delete ui;
}


QString Practice::get_tpc() {

    Global mGlobal;
    return mGlobal.get_textline(ivar::FILE_mn);
}


void Practice::get_icons_stats() {

    QSqlDatabase db = Database::instance().getConnection(tpc);
    QSqlQuery qry(db);

    for( int n = 1; n < 6; n = n + 1 ) {
        qry.prepare("SELECT * FROM Practice_icons");
        if (qry.exec( )) {
            while(qry.next()) {
                list_pair_icon_practs["pract1"]=qry.value(0).toString();
                list_pair_icon_practs["pract2"]=qry.value(1).toString();
                list_pair_icon_practs["pract3"]=qry.value(2).toString();
                list_pair_icon_practs["pract4"]=qry.value(3).toString();
                list_pair_icon_practs["pract5"]=qry.value(4).toString();
            }
        }
    }
    qry.finish();
}


void Practice::show_icons_stats() {

    tpc = get_tpc();

    this->setWindowTitle(tr("Practice - ")+tpc);

    ui->tableWidget->clearContents();
    ui->tableWidget->setRowCount(0);

    short unsigned n = 1;
    for ( auto it = list_pair_icon_practs.begin(); it != list_pair_icon_practs.end(); ++it  )
    {
       ui->tableWidget->insertRow(ui->tableWidget->rowCount());

       QString imgPath = ivar::DS+"/images/practice/"+it->second+".png";
       QImage *img = new QImage(imgPath);
       QTableWidgetItem *thumbnail = new QTableWidgetItem;
       thumbnail->setData(Qt::DecorationRole, QPixmap::fromImage(*img));

       ui->tableWidget->setItem(ui->tableWidget->rowCount() -1, 0, thumbnail);

       ui->tableWidget->setItem(ui->tableWidget->rowCount()-1 , 1, new QTableWidgetItem(practs[n-1]));

       n++;
    }
}


void Practice::starting_a_pract(QString pract) {

    active_pract = pract;

    list_easy.clear();
    list_learning.clear();
    list_learnt.clear();
    list_difficult.clear();

    QSqlDatabase db = Database::instance().getConnection(tpc);
    QSqlQuery qry(db);

    if (list_pair_icon_practs[active_pract] == "21") { // if the quiz is concluded --------------
        QMessageBox msgBox;
        msgBox.setText(tr("Practice Completed"));
        msgBox.setWindowTitle(tr("Practice Completed"));
        msgBox.setIcon(QMessageBox::Information);
        QPushButton *restartButton = msgBox.addButton(tr("Restart"), QMessageBox::ActionRole);
        QPushButton *okButton = msgBox.addButton(tr("Ok"), QMessageBox::ActionRole);
        msgBox.setDefaultButton(okButton);
        msgBox.exec();

        if (msgBox.clickedButton() == restartButton) {

            qry.exec("DELETE * FROM "+active_pract+" WHERE items_0");
            qry.exec("UPDATE Practice_icons SET "+active_pract+"='0'");

            practice_is_21_0(active_pract);
            list_pair_icon_practs[active_pract]="0";
            show_icons_stats();

            return;
        }
        else if (msgBox.clickedButton() == okButton) {

            return;
        }
    }

    else {

        if (list_pair_icon_practs[active_pract] == "0") {
           practice_is_21_0(active_pract);
       }

        qry.prepare("SELECT items_0 FROM "+active_pract+""); // if the quiz is started --------------

        this->hide();

        QString trgt, srce, type;

        count_total = 0;
        if (qry.exec( )) { // itinerate over items0 in Practx

            while(qry.next()) {

                trgt = qry.value(0).toString();
                QSqlQuery qry(db);
                qry.prepare("SELECT * FROM "+Source_LANG+" WHERE trgt=(:trgt_val)");
                qry.bindValue(":trgt_val", trgt);
                qry.exec( );
                qry.next();
                type = "2";
                srce = qry.value(1).toString();
                type = qry.value(13).toString();

                if (trgt != "" && srce != "" && type == "1") {

                    list_words.push_back(trgt);
                    list_pair_words[trgt]=srce;
                    count_total ++;
                }
            }
        }

        db.commit();
        qry.finish();

        if (active_pract == "pract1") {

            mPrac_a = new Prac_a();
            mPrac_a->load_data(list_pair_words,list_words);
            mPrac_a->show();
        }
        else if (active_pract == "pract2") {

            mPract2 = new Pract2();
            mPract2->load_data(list_pair_words,list_words);
            mPract2->show();

        }
        else if (active_pract == "pract3") {

            mPract3 = new Pract3();
            //mPract2->load_data(pair_words,words);
            mPract3->show();

        }
        else if (active_pract == "pract4") {

            mPract4 = new Pract4();
            //mPract2->load_data(pair_words,words);
            mPract4->show();
        }
        else if (active_pract == "pract5") {

            mPract5 = new Pract5();
            //mPract2->load_data(pair_words,words);
            mPract5->show();
        }
    }
}

void Practice::practice_is_21_0(QString active_pract) {

    QSqlDatabase db = Database::instance().getConnection(tpc);
    QSqlQuery qry(db);
    db.transaction();

    qry.prepare("SELECT list FROM learning");

    QString trgt, srce, type;
    if (qry.exec( )) {

        while(qry.next()) {
            trgt = qry.value(0).toString();

            QSqlQuery qry(db);
            qry.prepare("SELECT * FROM "+Source_LANG+" WHERE trgt=(:trgt_val)");
            qry.bindValue(":trgt_val", trgt);

            if (qry.exec( )) {
                while(qry.next()) {
                    srce = qry.value(1).toString();
                    type = qry.value(13).toString();
                }
            }
            if (trgt != "" && srce != "" && type == "1") {
                list_total.push_back(trgt);
            }
        }
    }

    for ( auto it = list_total.begin(); it != list_total.end(); ++it  ) {
        QString trgt = *it;
        qry.exec("insert into '"+active_pract+"' values ('"+trgt+"')");
    }

    qDebug() << "* New practice X session... Items:" << list_total.size();
    list_total.clear();
    db.commit();
    qry.finish();

}
void Practice::go_back_results(unsigned short int total,
                        std::vector< QString > &list_easy,
                        std::vector< QString > &list_learnt,
                        std::vector< QString > &list_learning,
                        std::vector< QString > &list_difficult,
                        QString active_pract) {

    QSqlDatabase db = Database::instance().getConnection(tpc);
    QSqlQuery qry(db);

    count_total = total;
    size_easy = list_easy.size();
    size_lrnt = list_learnt.size();
    size_ling = list_learning.size();
    size_hard = list_difficult.size();

    db.transaction();

    for ( auto it = list_easy.begin(); it != list_easy.end(); ++it  ) {
        if(!qry.exec("DELETE FROM '"+active_pract+"' WHERE items_0 = '"+*it+"'")) {
            qDebug() << qry.lastError().text();
        }
    }
    for ( auto it = list_learnt.begin(); it != list_learnt.end(); ++it  ) {
        if(!qry.exec("DELETE FROM '"+active_pract+"' WHERE items_0 = '"+*it+"'")) {
            qDebug() << qry.lastError().text();
        }
        qry.addBindValue(*it);
    }

    db.commit();
    qry.finish();

    calc_score_data(active_pract);
}


void Practice::calc_score_data(QString active_pract) {

    qDebug() << count_total
             << "|" << size_easy << "|" << size_lrnt
             << "|" << size_ling << "|" << size_hard;

    QString str_total = QString::number(count_total);

    QString str_easy = QString::number(size_easy);
    QString str_lrnt = QString::number(size_lrnt);
    QString str_ling = QString::number(size_ling);
    QString str_hard = QString::number(size_hard);

    unsigned long int isok = 100*size_easy/count_total;
    unsigned long int n = 1,  c = 1;

    while ( n <= 21 ) {
        if ( n == 21 ) {
            nicon_mod = QString::number(n-1);
            break;
        }
        else if ( isok <= c ) {
            nicon_mod = QString::number(n);
            break;
        }
        c = c + 5;
        n++;
    }

    list_pair_icon_practs[active_pract]=nicon_mod;

    ui->widget_score->show();
    ui->label_score_total->setText(str_total);
    ui->label_score_easy->setText(str_easy);
    ui->label_score_learnt->setText(str_lrnt);
    ui->label_score_learning->setText(str_ling);
    ui->label_score_hard->setText(str_hard);

    QSqlDatabase db = Database::instance().getConnection(tpc);
    QSqlQuery qry(db);

    qry.prepare("UPDATE Practice_icons SET "+active_pract+"='"+nicon_mod+"'");
    if (!qry.exec()) qDebug() << qry.lastError().text();
    qry.finish();

    show_icons_stats();
}


void Practice::on_pushButton_restart_clicked() {

    QSqlDatabase db = Database::instance().getConnection(tpc);
    QSqlQuery qry(db);
    db.transaction();
    if (!qry.exec("UPDATE Practice_icons SET Pract1=0")) qDebug() << qry.lastError().text();
    if (!qry.exec("UPDATE Practice_icons SET Pract2=0")) qDebug() << qry.lastError().text();
    if (!qry.exec("UPDATE Practice_icons SET Pract3=0")) qDebug() << qry.lastError().text();
    if (!qry.exec("UPDATE Practice_icons SET Pract4=0")) qDebug() << qry.lastError().text();
    if (!qry.exec("UPDATE Practice_icons SET Pract5=0")) qDebug() << qry.lastError().text();
    if(!qry.exec("DELETE FROM Pract1")) qDebug() << qry.lastError().text();
    if(!qry.exec("DELETE FROM Pract2")) qDebug() << qry.lastError().text();
    if(!qry.exec("DELETE FROM Pract3")) qDebug() << qry.lastError().text();
    if(!qry.exec("DELETE FROM Pract4")) qDebug() << qry.lastError().text();
    if(!qry.exec("DELETE FROM Pract5")) qDebug() << qry.lastError().text();
    list_pair_icon_practs["pract1"]="0";
    list_pair_icon_practs["pract2"]="0";
    list_pair_icon_practs["pract3"]="0";
    list_pair_icon_practs["pract4"]="0";
    list_pair_icon_practs["pract5"]="0";
    db.commit();

    show_icons_stats();
    ui->pushButton_restart->setEnabled(false);
}


void Practice::on_tableWidget_itemDoubleClicked(QTableWidgetItem *item) { // item->text();

    starting_a_pract("pract"+QString::number(item->row()+1));
}


void Practice::save_data() {

    // ------------------------------icons stats
}


void Practice::closeEvent(QCloseEvent * event) {

    if(this->isVisible()){
        event->ignore();
        this->hide();
    }

    save_data();
}
