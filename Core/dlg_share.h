#ifndef DLG_SHARE_H
#define DLG_SHARE_H

#include <QDialog>

namespace Ui {
class Share;
}

class Share : public QDialog
{
    Q_OBJECT

public:
    explicit Share(QWidget *parent = nullptr);
    ~Share();
    void load_data();
    QString get_tpc();
    QString tpc;

private slots:
    void on_pushButton_close_clicked();
    void on_pushButton_upload_clicked();
    void on_pushButton_export_clicked();

private:
    Ui::Share *ui;
};

#endif // DLG_SHARE_H
