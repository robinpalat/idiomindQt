#ifndef DLG_PRACTICE_H
#define DLG_PRACTICE_H

#include <QDialog>

namespace Ui {
class Practice;
}

class Practice : public QDialog
{
    Q_OBJECT

public:
    explicit Practice(QWidget *parent = nullptr);
    ~Practice();

public:
    void load_data();

private:
    Ui::Practice *ui;
};

#endif // DLG_PRACTICE_H
