#ifndef DLG_ADD_H
#define DLG_ADD_H

#include <QDialog>

namespace Ui {
class Add;
}

class Add : public QDialog
{
    Q_OBJECT

public:
    explicit Add(QWidget *parent = nullptr);
    ~Add();

    bool field_srce;
    void load_data();

private slots:
    void on_pushButton_add_clicked();
    void on_pushButton_more_clicked();
    void on_pushButton_audio_clicked();
    void on_pushButton_image_clicked();
    void on_pushButton_clip_clicked();
    void on_pushButton_srce_clicked();
    void closeEvent(QCloseEvent * event);

private:
    Ui::Add *ui;
};

#endif // DLG_ADD_H
