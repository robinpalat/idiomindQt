
#include "dlg_translate.h"
#include "ui_dlg_translate.h"
#include "vars_session.h"

#include <QtWidgets/QtWidgets>
#include <QGroupBox>
#include <QFormLayout>
#include <QLineEdit>


Translate::Translate(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Translate)
{
    ui->setupUi(this);
    ui->widget_verified->hide();
    load_data();
}

Translate::~Translate()
{
    delete ui;
}

QString Translate::get_tpc() {
    Global mGlobal;
    return mGlobal.get_textline(ivar::FILE_mn);
}


// laod list of items
void Translate::load_data() {

    tpc = get_tpc();

    QSqlDatabase mydb;

    this->setWindowTitle("Idiomind - "+tpc);

    mydb.setDatabaseName(ivar::DM_tl+"/"+tpc+"/.conf/tpcdb");

    mydb=QSqlDatabase::addDatabase("QSQLITE");
    mydb.setDatabaseName(ivar::DM_tl+"/"+tpc+"/.conf/tpcdb");

    if (!mydb.open()) qDebug()<<("Failed to open the database");

    QSqlQuery* qry_a=new QSqlQuery(mydb);

    qry_a->prepare("select * from Data");
    //QFormLayout *layouta = new QFormLayout;
    int n = 0;
    if (qry_a->exec( )) {
        while(qry_a->next()) {

            QString trgt = qry_a->value(0).toString();
            QString srce = qry_a->value(1).toString();
            trans_load_items.push_back(srce);

            //QGroupBox * BOX = new QGroupBox(QString::number(n), this);

            QFormLayout * layoutA = new QFormLayout;
            QFormLayout * layoutB = new QFormLayout;

            QSpacerItem * ES1 = new QSpacerItem(10,10);

            QLabel * LA = new QLabel(trgt);
            LA->setObjectName("LA"+QString::number(n));
            LA->setMaximumWidth(488);
            LA->setMinimumHeight(50);
            LA->setStyleSheet(QString("font-weight: bold; color: #14273E ;background-color: #E7F0F9;"));
            LA->setMargin(6);
            LA->setAlignment(Qt::AlignLeft | Qt::AlignTop);
            LA->setWordWrap(488);
            layoutA->addWidget(LA);

            QTextEdit * TE = new QTextEdit(srce);
            TE->setObjectName("TE"+QString::number(n));
            TE->setMaximumWidth(488);
            TE->setMaximumHeight(50);
            TE->setFrameStyle(QFrame::Plain);
            TE->setStyleSheet("background-color: #F7F8FC; color: #32393E;");

            layoutA->addWidget(TE);
            layoutA->setVerticalSpacing(0);
            //layoutA->setHorizontalSpacing(0);

//            QSpacerItem * ES2 = new QSpacerItem(10,10);
//            layoutB->addItem(ES2);

//            QWidget *LI = new QWidget;
//            LI->setFixedHeight(2);
//            LI->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
//            LI->setStyleSheet(QString("background-color: #E0E6ED;"));
//            layoutB->addWidget(LI);

            //BOX->setLayout(layoutA);


            layoutB->addRow(layoutA);

            layoutB->addItem(ES1);

            ui->layouta->addLayout(layoutB);

            n++;
        }
    }
    //ui->layouta->setHorizontalSpacing(0);
    ui->scrollAreaWidgetContents->setLayout(ui->layouta);
    ui->scrollAreaWidgetContents->setStyleSheet("border-color: #F8FAFC;");

    for(int i = 0; i < 47; i++) {
        ui->comboBox_nativelang->addItem(ivar::slangs[0][i]);
        if (ivar::slangs[0][i] == ivar::slng) lgs = ivar::slangs[1][i];
    }

    ui->label_nativeLang->setText(ivar::slng);
    ui->checkBox_verifiedtrans->setText("Verified — "+ivar::slng+"");

    // check/load verified languages
    qry_a->prepare("select * from verified_translations");
    ui->comboBox_verified_trans->addItem("");

    //
    lang_verif_count = 0;
    if (qry_a->exec( )) {
        while(qry_a->next()) {
            QString lang = qry_a->value(0).toString();

            // Adding item to first comboBox list of veried translations avoiding current language
            if ( lang != "" && lang != ivar::slng ) {
                ui->comboBox_verified_trans->addItem(lang);
                lang_verif_count++;
            }
            // checkBok, true if current native language was verified by user
            if ( lang == ivar::slng)  {
                current_lang_is_already_verified = true;
                ui->checkBox_verifiedtrans->setText("The translation was verified — "+ivar::slng+"");
                ui->checkBox_verifiedtrans->setChecked(true);
            }
        }
    }

    if (current_lang_is_already_verified == false) {
        ui->checkBox_verifiedtrans->setText("Verified — "+ivar::slng+"");
        ui->checkBox_verifiedtrans->setChecked(false);
    }

    if ( lang_verif_count > 0 ) {
        ui->widget_no_verified->hide();
        ui->widget_verified->show();
    }
    else {
        ui->widget_no_verified->show();
        ui->widget_verified->hide();
    }
}


void Translate::on_pushButton_close_clicked()
{
    this->close();
}


void Translate::save_data() {

    QSqlDatabase mydb;

    this->setWindowTitle("Idiomind - "+tpc);

    mydb.setDatabaseName(ivar::DM_tl+"/"+tpc+"/.conf/tpcdb");

    mydb=QSqlDatabase::addDatabase("QSQLITE");
    mydb.setDatabaseName(ivar::DM_tl+"/"+tpc+"/.conf/tpcdb");

    if (!mydb.open()) qDebug()<<("Failed to open the database");

    trans_check_items.clear();
    // getting values from orphated widget this qobject_cast, usind nested calls
    for(int i = 0; i < ui->layouta->count(); i++){
        QFormLayout * mQFormLayoutA = qobject_cast<QFormLayout*>(ui->layouta->itemAt(i)->layout());
        QFormLayout * mQFormLayoutB = qobject_cast<QFormLayout*>(mQFormLayoutA->itemAt(0)->layout());
        QTextEdit *item = qobject_cast<QTextEdit*>(mQFormLayoutB->itemAt(1)->widget());
        trans_check_items.push_back(item->toPlainText());
    }

    if (trans_load_items != trans_check_items ) {

        QString* arraytrgt = &trans_check_items[0];

        int n = 0;
        for(std::vector<QString>::iterator it = trans_load_items.begin(); it != trans_load_items.end(); ++it) {
            if (*it != arraytrgt[n]) {
                QSqlQuery qry;
                qry.prepare("update Data set srce='"+arraytrgt[n]+"' where srce='"+*it+"'");
                qry.exec();
            }
            n++;
        }
    }

    mydb.close();
    mydb = QSqlDatabase();
    mydb.removeDatabase(QSqlDatabase::defaultConnection);
}


void Translate::on_pushButton_save_trans_clicked()
{
    save_data();
    ui->pushButton_save_trans->setEnabled(false);
}


void Translate::on_checkBox_verifiedtrans_stateChanged(int arg1)
{
    QSqlQuery qry_c;

    if (arg1==0 && current_lang_is_already_verified == true) {

            ui->checkBox_verifiedtrans->setText("Verified — "+ivar::slng+"");

            qry_c.prepare("DELETE FROM verified_translations WHERE lang = ?");
            qry_c.addBindValue(ivar::slng);
            if (!qry_c.exec()) qDebug() << qry_c.lastError();
    }
     else {
        if (current_lang_is_already_verified == false) {

            ui->checkBox_verifiedtrans->setText("The translation was verified — "+ivar::slng+"");

            qry_c.prepare("INSERT INTO verified_translations (lang) values (?)");
            qry_c.addBindValue(ivar::slng);
            if (!qry_c.exec()) qDebug() << qry_c.lastError();
        }
    }

    if ( lang_verif_count > 0 ) {
        ui->widget_no_verified->hide();
        ui->widget_verified->show();
    }
    else {
        ui->widget_no_verified->show();
        ui->widget_verified->hide();
    }

        /*

        qry_c.prepare("update verified_translations set "+lgs+"=1 where "+lgs+"=0");

        */
}
