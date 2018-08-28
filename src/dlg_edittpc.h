#ifndef DLG_EDITTPC_H
#define DLG_EDITTPC_H

#include <QDialog>
#include <QSql>

#include "vars_statics.h"
#include "vars_session.h"

namespace Ui {
class EditTpc;
}

class EditTpc : public QDialog
{
    Q_OBJECT

public:
    explicit EditTpc(QWidget *parent = nullptr);
    ~EditTpc();

    void load_data();
    void save_data();
    QString get_tpc();
    QString tpc;
    int numberOfRows = 0, numberOfRowsx = 0;
    std::vector< QString > edittpc_load_items;
    std::vector< QString > edittpc_check_items;


private slots:

    void on_pushButton_close_clicked();
    void on_pushButton_save_edits_clicked();

private:
    Ui::EditTpc *ui;
};

#endif // DLG_EDITTPC_H
