#ifndef DLG_TRANSLATE_H
#define DLG_TRANSLATE_H

#include <QDialog>
#include <QSql>
#include <QTableWidgetItem>
#include <QGroupBox>
#include <QFormLayout>
#include <QLineEdit>
#include <QTextEdit>

#include "vars_statics.h"
#include "vars_session.h"
#include "Media/database.h"

namespace Ui {
class Translate;
}

class Translate : public QDialog
{
    Q_OBJECT

public:
    explicit Translate(QWidget *parent = nullptr);
    ~Translate();

public:
    void load_data();
    void save_data();
    QString get_tpc();
    QString tpc, lgs;
    bool current_lang_is_already_verified = false;
    int lang_verif_count;
    std::vector< QString > trans_load_items;
    std::vector< QString > trans_check_items;


private slots:
    void on_pushButton_close_clicked();
    void on_pushButton_save_trans_clicked();
    void on_checkBox_verifiedtrans_stateChanged(int arg1);

private:
    Ui::Translate *ui;
    QVBoxLayout *vbox;
};

#endif // DLG_TRANSLATE_H
