/********************************************************************************
** Form generated from reading UI file 'prac_a.ui'
**
** Created by: Qt User Interface Compiler version 5.10.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_PRAC_A_H
#define UI_PRAC_A_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QFrame>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_Prac_a
{
public:
    QGridLayout *gridLayout;
    QWidget *widget;
    QVBoxLayout *verticalLayout;
    QLabel *label_trgt;
    QHBoxLayout *horizontalLayout_2;
    QSpacerItem *horizontalSpacer_3;
    QFrame *line;
    QSpacerItem *horizontalSpacer_2;
    QLabel *label_srce;
    QSpacerItem *verticalSpacer;
    QHBoxLayout *horizontalLayout;
    QPushButton *pushButton_exit;
    QPushButton *pushButton_nex;

    void setupUi(QWidget *Prac_a)
    {
        if (Prac_a->objectName().isEmpty())
            Prac_a->setObjectName(QStringLiteral("Prac_a"));
        Prac_a->resize(544, 321);
        Prac_a->setMinimumSize(QSize(450, 0));
        QIcon icon;
        icon.addFile(QStringLiteral(":/images/idiomind.png"), QSize(), QIcon::Normal, QIcon::Off);
        Prac_a->setWindowIcon(icon);
        gridLayout = new QGridLayout(Prac_a);
        gridLayout->setObjectName(QStringLiteral("gridLayout"));
        gridLayout->setHorizontalSpacing(15);
        gridLayout->setContentsMargins(12, 6, 15, 15);
        widget = new QWidget(Prac_a);
        widget->setObjectName(QStringLiteral("widget"));
        widget->setMinimumSize(QSize(450, 240));
        verticalLayout = new QVBoxLayout(widget);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        label_trgt = new QLabel(widget);
        label_trgt->setObjectName(QStringLiteral("label_trgt"));
        QFont font;
        font.setPointSize(26);
        font.setBold(true);
        font.setWeight(75);
        label_trgt->setFont(font);
        label_trgt->setAlignment(Qt::AlignCenter);

        verticalLayout->addWidget(label_trgt);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setObjectName(QStringLiteral("horizontalLayout_2"));
        horizontalLayout_2->setContentsMargins(-1, 6, -1, 6);
        horizontalSpacer_3 = new QSpacerItem(40, 20, QSizePolicy::Minimum, QSizePolicy::Minimum);

        horizontalLayout_2->addItem(horizontalSpacer_3);

        line = new QFrame(widget);
        line->setObjectName(QStringLiteral("line"));
        line->setStyleSheet(QStringLiteral("color: rgb(171, 171, 171);"));
        line->setFrameShadow(QFrame::Sunken);
        line->setFrameShape(QFrame::HLine);

        horizontalLayout_2->addWidget(line);

        horizontalSpacer_2 = new QSpacerItem(40, 20, QSizePolicy::Minimum, QSizePolicy::Minimum);

        horizontalLayout_2->addItem(horizontalSpacer_2);


        verticalLayout->addLayout(horizontalLayout_2);

        label_srce = new QLabel(widget);
        label_srce->setObjectName(QStringLiteral("label_srce"));
        QFont font1;
        font1.setPointSize(18);
        font1.setBold(true);
        font1.setItalic(true);
        font1.setWeight(75);
        label_srce->setFont(font1);
        label_srce->setAlignment(Qt::AlignHCenter|Qt::AlignTop);

        verticalLayout->addWidget(label_srce);

        verticalSpacer = new QSpacerItem(20, 80, QSizePolicy::Minimum, QSizePolicy::Fixed);

        verticalLayout->addItem(verticalSpacer);


        gridLayout->addWidget(widget, 0, 0, 1, 1);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setSpacing(12);
        horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
        horizontalLayout->setContentsMargins(12, -1, 12, -1);
        pushButton_exit = new QPushButton(Prac_a);
        pushButton_exit->setObjectName(QStringLiteral("pushButton_exit"));
        pushButton_exit->setMinimumSize(QSize(80, 28));

        horizontalLayout->addWidget(pushButton_exit);

        pushButton_nex = new QPushButton(Prac_a);
        pushButton_nex->setObjectName(QStringLiteral("pushButton_nex"));
        pushButton_nex->setMinimumSize(QSize(80, 28));

        horizontalLayout->addWidget(pushButton_nex);


        gridLayout->addLayout(horizontalLayout, 1, 0, 1, 1);


        retranslateUi(Prac_a);

        QMetaObject::connectSlotsByName(Prac_a);
    } // setupUi

    void retranslateUi(QWidget *Prac_a)
    {
        Prac_a->setWindowTitle(QApplication::translate("Prac_a", "Flashcards", nullptr));
        label_trgt->setText(QApplication::translate("Prac_a", "TextLabel", nullptr));
        label_srce->setText(QApplication::translate("Prac_a", "TextLabel", nullptr));
        pushButton_exit->setText(QString());
        pushButton_nex->setText(QString());
    } // retranslateUi

};

namespace Ui {
    class Prac_a: public Ui_Prac_a {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_PRAC_A_H
