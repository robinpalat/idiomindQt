#ifndef DLG_IMAGEVIEW_H
#define DLG_IMAGEVIEW_H

#include <QDialog>
#include "vars_session.h"

namespace Ui {
class Dlg_ImageView;
}

class Dlg_ImageView : public QDialog
{
    Q_OBJECT

public:
    explicit Dlg_ImageView(QWidget *parent = nullptr);
    ~Dlg_ImageView();

    void load_image(QString trgt);

private:
    Ui::Dlg_ImageView *ui;
};

#endif // DLG_IMAGEVIEW_H
