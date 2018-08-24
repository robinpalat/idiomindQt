#ifndef DLG_EDITTPC_H
#define DLG_EDITTPC_H

#include <QDialog>
#include <QSql>

#include "session.h"
#include "global.h"

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
    QString get_tpc();
    QString tpc;

private slots:
    void on_pushButton_cancel_clicked();
    void on_pushButton_save_clicked();
    void on_pushButton_more_clicked();
    void on_pushButton_close_clicked();

    void on_pushButton_save_edits_clicked();

private:
    Ui::EditTpc *ui;
};

#endif // DLG_EDITTPC_H
