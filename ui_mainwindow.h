/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 5.10.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QTableView>
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QAction *actionAbout;
    QAction *actionGetting_Started;
    QAction *actionFeedback;
    QAction *actionSeetings;
    QAction *actionOpen;
    QAction *actionImport;
    QAction *actionExport;
    QWidget *centralwidget;
    QWidget *centralWidget;
    QVBoxLayout *verticalLayout;
    QTabWidget *tabWidget;
    QWidget *Learning;
    QGridLayout *gridLayout;
    QTableWidget *list_learning;
    QWidget *Learnt;
    QGridLayout *gridLayout_2;
    QTableView *list_learnt;
    QWidget *Notes;
    QGridLayout *gridLayout_3;
    QTextEdit *textEdit;
    QWidget *Manage;
    QGroupBox *groupBox_3;
    QWidget *layoutWidget;
    QVBoxLayout *verticalLayout_2;
    QPushButton *pushButton_tabmanage_edit;
    QPushButton *pushButton_tabmanage_trans;
    QPushButton *pushButton_tabmanage_share;
    QPushButton *pushButton_tabmanage_delete;
    QWidget *layoutWidget_2;
    QVBoxLayout *verticalLayout_5;
    QHBoxLayout *horizontalLayout_3;
    QLabel *label_4;
    QLineEdit *lineEdit_tabmanage_rename;
    QPushButton *pushButton_tabmanage_markas;
    QWidget *layoutWidget_3;
    QVBoxLayout *verticalLayout_6;
    QLabel *label_tpc_name;
    QLabel *label_tabmanage_info;
    QHBoxLayout *horizontalLayout;
    QSpacerItem *horizontalSpacer;
    QPushButton *pushButton_play;
    QPushButton *pushButton_practice;
    QPushButton *pushButton_close;
    QMenuBar *menubar;
    QMenu *menuFile;
    QMenu *menuHelp;
    QMenu *menuOptios;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QStringLiteral("MainWindow"));
        MainWindow->resize(572, 572);
        actionAbout = new QAction(MainWindow);
        actionAbout->setObjectName(QStringLiteral("actionAbout"));
        actionGetting_Started = new QAction(MainWindow);
        actionGetting_Started->setObjectName(QStringLiteral("actionGetting_Started"));
        actionFeedback = new QAction(MainWindow);
        actionFeedback->setObjectName(QStringLiteral("actionFeedback"));
        actionSeetings = new QAction(MainWindow);
        actionSeetings->setObjectName(QStringLiteral("actionSeetings"));
        actionOpen = new QAction(MainWindow);
        actionOpen->setObjectName(QStringLiteral("actionOpen"));
        actionImport = new QAction(MainWindow);
        actionImport->setObjectName(QStringLiteral("actionImport"));
        actionExport = new QAction(MainWindow);
        actionExport->setObjectName(QStringLiteral("actionExport"));
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName(QStringLiteral("centralwidget"));
        centralWidget = new QWidget(centralwidget);
        centralWidget->setObjectName(QStringLiteral("centralWidget"));
        centralWidget->setGeometry(QRect(10, 0, 551, 541));
        verticalLayout = new QVBoxLayout(centralWidget);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        verticalLayout->setSizeConstraint(QLayout::SetMinimumSize);
        verticalLayout->setContentsMargins(0, 0, 0, 0);
        tabWidget = new QTabWidget(centralWidget);
        tabWidget->setObjectName(QStringLiteral("tabWidget"));
        QFont font;
        font.setPointSize(10);
        tabWidget->setFont(font);
        tabWidget->setMouseTracking(true);
        tabWidget->setTabletTracking(true);
        tabWidget->setAcceptDrops(true);
        tabWidget->setDocumentMode(false);
        tabWidget->setMovable(false);
        Learning = new QWidget();
        Learning->setObjectName(QStringLiteral("Learning"));
        gridLayout = new QGridLayout(Learning);
        gridLayout->setObjectName(QStringLiteral("gridLayout"));
        gridLayout->setContentsMargins(10, 10, 10, 10);
        list_learning = new QTableWidget(Learning);
        list_learning->setObjectName(QStringLiteral("list_learning"));
        list_learning->setMinimumSize(QSize(520, 450));
        list_learning->setMaximumSize(QSize(540, 500));
        list_learning->setFont(font);
        list_learning->setAcceptDrops(true);
        list_learning->setFrameShape(QFrame::NoFrame);
        list_learning->setFrameShadow(QFrame::Plain);
        list_learning->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        list_learning->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        list_learning->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
        list_learning->setAutoScroll(false);
        list_learning->setEditTriggers(QAbstractItemView::NoEditTriggers);
        list_learning->setTabKeyNavigation(false);
        list_learning->setProperty("showDropIndicator", QVariant(false));
        list_learning->setDragEnabled(false);
        list_learning->setSelectionMode(QAbstractItemView::SingleSelection);
        list_learning->setSelectionBehavior(QAbstractItemView::SelectRows);
        list_learning->setIconSize(QSize(5, 5));
        list_learning->setShowGrid(false);
        list_learning->setGridStyle(Qt::NoPen);
        list_learning->setCornerButtonEnabled(false);
        list_learning->setRowCount(0);
        list_learning->horizontalHeader()->setVisible(false);
        list_learning->horizontalHeader()->setCascadingSectionResizes(false);
        list_learning->horizontalHeader()->setDefaultSectionSize(500);
        list_learning->horizontalHeader()->setHighlightSections(false);
        list_learning->horizontalHeader()->setProperty("showSortIndicator", QVariant(false));
        list_learning->verticalHeader()->setVisible(false);
        list_learning->verticalHeader()->setCascadingSectionResizes(false);
        list_learning->verticalHeader()->setHighlightSections(false);

        gridLayout->addWidget(list_learning, 0, 0, 1, 1);

        tabWidget->addTab(Learning, QString());
        Learnt = new QWidget();
        Learnt->setObjectName(QStringLiteral("Learnt"));
        gridLayout_2 = new QGridLayout(Learnt);
        gridLayout_2->setObjectName(QStringLiteral("gridLayout_2"));
        gridLayout_2->setContentsMargins(10, 10, 10, 10);
        list_learnt = new QTableView(Learnt);
        list_learnt->setObjectName(QStringLiteral("list_learnt"));
        list_learnt->setFont(font);
        list_learnt->setFrameShape(QFrame::HLine);
        list_learnt->setFrameShadow(QFrame::Plain);
        list_learnt->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        list_learnt->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        list_learnt->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
        list_learnt->setAutoScroll(false);
        list_learnt->setTabKeyNavigation(false);
        list_learnt->setProperty("showDropIndicator", QVariant(false));
        list_learnt->setSelectionMode(QAbstractItemView::SingleSelection);
        list_learnt->setShowGrid(false);
        list_learnt->setGridStyle(Qt::NoPen);
        list_learnt->setCornerButtonEnabled(false);
        list_learnt->horizontalHeader()->setVisible(false);
        list_learnt->horizontalHeader()->setDefaultSectionSize(500);
        list_learnt->horizontalHeader()->setHighlightSections(false);
        list_learnt->horizontalHeader()->setMinimumSectionSize(500);
        list_learnt->verticalHeader()->setVisible(false);
        list_learnt->verticalHeader()->setHighlightSections(false);

        gridLayout_2->addWidget(list_learnt, 0, 0, 1, 1);

        tabWidget->addTab(Learnt, QString());
        Notes = new QWidget();
        Notes->setObjectName(QStringLiteral("Notes"));
        gridLayout_3 = new QGridLayout(Notes);
        gridLayout_3->setObjectName(QStringLiteral("gridLayout_3"));
        textEdit = new QTextEdit(Notes);
        textEdit->setObjectName(QStringLiteral("textEdit"));
        QFont font1;
        font1.setPointSize(11);
        textEdit->setFont(font1);
        textEdit->setStyleSheet(QLatin1String("\n"
"background-color: rgb(255, 255, 249);\n"
"color: rgb(61, 61, 61);"));
        textEdit->setFrameShape(QFrame::NoFrame);
        textEdit->setLineWidth(0);
        textEdit->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

        gridLayout_3->addWidget(textEdit, 0, 0, 1, 1);

        tabWidget->addTab(Notes, QString());
        Manage = new QWidget();
        Manage->setObjectName(QStringLiteral("Manage"));
        Manage->setMinimumSize(QSize(521, 467));
        groupBox_3 = new QGroupBox(Manage);
        groupBox_3->setObjectName(QStringLiteral("groupBox_3"));
        groupBox_3->setGeometry(QRect(10, -10, 531, 481));
        layoutWidget = new QWidget(groupBox_3);
        layoutWidget->setObjectName(QStringLiteral("layoutWidget"));
        layoutWidget->setGeometry(QRect(410, 90, 101, 151));
        verticalLayout_2 = new QVBoxLayout(layoutWidget);
        verticalLayout_2->setObjectName(QStringLiteral("verticalLayout_2"));
        verticalLayout_2->setContentsMargins(0, 0, 0, 0);
        pushButton_tabmanage_edit = new QPushButton(layoutWidget);
        pushButton_tabmanage_edit->setObjectName(QStringLiteral("pushButton_tabmanage_edit"));
        pushButton_tabmanage_edit->setFlat(false);

        verticalLayout_2->addWidget(pushButton_tabmanage_edit);

        pushButton_tabmanage_trans = new QPushButton(layoutWidget);
        pushButton_tabmanage_trans->setObjectName(QStringLiteral("pushButton_tabmanage_trans"));
        pushButton_tabmanage_trans->setFlat(false);

        verticalLayout_2->addWidget(pushButton_tabmanage_trans);

        pushButton_tabmanage_share = new QPushButton(layoutWidget);
        pushButton_tabmanage_share->setObjectName(QStringLiteral("pushButton_tabmanage_share"));
        pushButton_tabmanage_share->setFlat(false);

        verticalLayout_2->addWidget(pushButton_tabmanage_share);

        pushButton_tabmanage_delete = new QPushButton(layoutWidget);
        pushButton_tabmanage_delete->setObjectName(QStringLiteral("pushButton_tabmanage_delete"));
        QSizePolicy sizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(pushButton_tabmanage_delete->sizePolicy().hasHeightForWidth());
        pushButton_tabmanage_delete->setSizePolicy(sizePolicy);
        pushButton_tabmanage_delete->setMinimumSize(QSize(0, 26));

        verticalLayout_2->addWidget(pushButton_tabmanage_delete);

        layoutWidget_2 = new QWidget(groupBox_3);
        layoutWidget_2->setObjectName(QStringLiteral("layoutWidget_2"));
        layoutWidget_2->setGeometry(QRect(10, 90, 371, 60));
        verticalLayout_5 = new QVBoxLayout(layoutWidget_2);
        verticalLayout_5->setObjectName(QStringLiteral("verticalLayout_5"));
        verticalLayout_5->setContentsMargins(0, 0, 0, 0);
        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setObjectName(QStringLiteral("horizontalLayout_3"));
        label_4 = new QLabel(layoutWidget_2);
        label_4->setObjectName(QStringLiteral("label_4"));

        horizontalLayout_3->addWidget(label_4);

        lineEdit_tabmanage_rename = new QLineEdit(layoutWidget_2);
        lineEdit_tabmanage_rename->setObjectName(QStringLiteral("lineEdit_tabmanage_rename"));

        horizontalLayout_3->addWidget(lineEdit_tabmanage_rename);


        verticalLayout_5->addLayout(horizontalLayout_3);

        pushButton_tabmanage_markas = new QPushButton(layoutWidget_2);
        pushButton_tabmanage_markas->setObjectName(QStringLiteral("pushButton_tabmanage_markas"));
        pushButton_tabmanage_markas->setMinimumSize(QSize(0, 26));

        verticalLayout_5->addWidget(pushButton_tabmanage_markas);

        layoutWidget_3 = new QWidget(groupBox_3);
        layoutWidget_3->setObjectName(QStringLiteral("layoutWidget_3"));
        layoutWidget_3->setGeometry(QRect(10, 30, 501, 46));
        verticalLayout_6 = new QVBoxLayout(layoutWidget_3);
        verticalLayout_6->setObjectName(QStringLiteral("verticalLayout_6"));
        verticalLayout_6->setContentsMargins(0, 0, 0, 0);
        label_tpc_name = new QLabel(layoutWidget_3);
        label_tpc_name->setObjectName(QStringLiteral("label_tpc_name"));
        QFont font2;
        font2.setPointSize(16);
        label_tpc_name->setFont(font2);

        verticalLayout_6->addWidget(label_tpc_name);

        label_tabmanage_info = new QLabel(layoutWidget_3);
        label_tabmanage_info->setObjectName(QStringLiteral("label_tabmanage_info"));

        verticalLayout_6->addWidget(label_tabmanage_info);

        tabWidget->addTab(Manage, QString());

        verticalLayout->addWidget(tabWidget);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
        horizontalLayout->setContentsMargins(0, -1, 10, -1);
        horizontalSpacer = new QSpacerItem(250, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer);

        pushButton_play = new QPushButton(centralWidget);
        pushButton_play->setObjectName(QStringLiteral("pushButton_play"));
        QSizePolicy sizePolicy1(QSizePolicy::Minimum, QSizePolicy::Minimum);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(pushButton_play->sizePolicy().hasHeightForWidth());
        pushButton_play->setSizePolicy(sizePolicy1);
        pushButton_play->setMinimumSize(QSize(0, 26));
        pushButton_play->setAutoDefault(false);
        pushButton_play->setFlat(false);

        horizontalLayout->addWidget(pushButton_play);

        pushButton_practice = new QPushButton(centralWidget);
        pushButton_practice->setObjectName(QStringLiteral("pushButton_practice"));
        sizePolicy1.setHeightForWidth(pushButton_practice->sizePolicy().hasHeightForWidth());
        pushButton_practice->setSizePolicy(sizePolicy1);
        pushButton_practice->setMinimumSize(QSize(0, 26));
        pushButton_practice->setAutoDefault(false);
        pushButton_practice->setFlat(false);

        horizontalLayout->addWidget(pushButton_practice);

        pushButton_close = new QPushButton(centralWidget);
        pushButton_close->setObjectName(QStringLiteral("pushButton_close"));
        sizePolicy1.setHeightForWidth(pushButton_close->sizePolicy().hasHeightForWidth());
        pushButton_close->setSizePolicy(sizePolicy1);
        pushButton_close->setMinimumSize(QSize(0, 26));
        pushButton_close->setLayoutDirection(Qt::LeftToRight);
        pushButton_close->setFlat(false);

        horizontalLayout->addWidget(pushButton_close);

        horizontalLayout->setStretch(1, 1);
        horizontalLayout->setStretch(2, 1);
        horizontalLayout->setStretch(3, 1);

        verticalLayout->addLayout(horizontalLayout);

        MainWindow->setCentralWidget(centralwidget);
        menubar = new QMenuBar(MainWindow);
        menubar->setObjectName(QStringLiteral("menubar"));
        menubar->setGeometry(QRect(0, 0, 572, 19));
        menubar->setNativeMenuBar(true);
        menuFile = new QMenu(menubar);
        menuFile->setObjectName(QStringLiteral("menuFile"));
        menuHelp = new QMenu(menubar);
        menuHelp->setObjectName(QStringLiteral("menuHelp"));
        menuOptios = new QMenu(menubar);
        menuOptios->setObjectName(QStringLiteral("menuOptios"));
        MainWindow->setMenuBar(menubar);

        menubar->addAction(menuFile->menuAction());
        menubar->addAction(menuOptios->menuAction());
        menubar->addAction(menuHelp->menuAction());
        menuFile->addAction(actionOpen);
        menuFile->addAction(actionImport);
        menuFile->addAction(actionExport);
        menuHelp->addAction(actionGetting_Started);
        menuHelp->addAction(actionFeedback);
        menuHelp->addAction(actionAbout);
        menuOptios->addAction(actionSeetings);

        retranslateUi(MainWindow);

        tabWidget->setCurrentIndex(0);
        pushButton_close->setDefault(true);


        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QApplication::translate("MainWindow", "MainWindow", nullptr));
        actionAbout->setText(QApplication::translate("MainWindow", "About", nullptr));
        actionGetting_Started->setText(QApplication::translate("MainWindow", "Getting Started", nullptr));
        actionFeedback->setText(QApplication::translate("MainWindow", "Feedback", nullptr));
        actionSeetings->setText(QApplication::translate("MainWindow", "Seetings", nullptr));
        actionOpen->setText(QApplication::translate("MainWindow", "Open", nullptr));
        actionImport->setText(QApplication::translate("MainWindow", "Import", nullptr));
        actionExport->setText(QApplication::translate("MainWindow", "Export", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(Learning), QApplication::translate("MainWindow", "Learning", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(Learnt), QApplication::translate("MainWindow", "Learnt", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(Notes), QApplication::translate("MainWindow", "Notes", nullptr));
        groupBox_3->setTitle(QString());
        pushButton_tabmanage_edit->setText(QApplication::translate("MainWindow", "Edit", nullptr));
        pushButton_tabmanage_trans->setText(QApplication::translate("MainWindow", "Translate", nullptr));
        pushButton_tabmanage_share->setText(QApplication::translate("MainWindow", "Share", nullptr));
        pushButton_tabmanage_delete->setText(QApplication::translate("MainWindow", "Delete", nullptr));
        label_4->setText(QApplication::translate("MainWindow", "Rename", nullptr));
        pushButton_tabmanage_markas->setText(QApplication::translate("MainWindow", "Mark as Learned", nullptr));
        label_tpc_name->setText(QApplication::translate("MainWindow", "Topic Name", nullptr));
        label_tabmanage_info->setText(QApplication::translate("MainWindow", "Topic info", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(Manage), QApplication::translate("MainWindow", "Manage", nullptr));
        pushButton_play->setText(QApplication::translate("MainWindow", "Play", nullptr));
        pushButton_practice->setText(QApplication::translate("MainWindow", "Practice", nullptr));
        pushButton_close->setText(QApplication::translate("MainWindow", "Close", nullptr));
        menuFile->setTitle(QApplication::translate("MainWindow", "File", nullptr));
        menuHelp->setTitle(QApplication::translate("MainWindow", "Help", nullptr));
        menuOptios->setTitle(QApplication::translate("MainWindow", "Optios", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
