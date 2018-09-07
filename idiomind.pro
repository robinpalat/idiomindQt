#-------------------------------------------------
#
# Project created by QtCreator 2018-07-14T15:52:03
#
#-------------------------------------------------

QT       += core gui sql multimedia


greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = idiomind
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += \
        Core/main.cpp \
        Core/icontray.cpp \
        Core/item_list.cpp \
    Core/dlg_imageview.cpp \
    Core/dlg_vwr.cpp \
    Core/dlg_edititem.cpp \
    Core/dlg_topics.cpp \
    Core/welcome.cpp \
    Core/dlg_share.cpp \
    Core/dlg_edittpc.cpp \
    Core/dlg_translate.cpp \
    Core/mainwindow.cpp \
    Core/vars_statics.cpp \
    Core/vars_session.cpp \
    Classes/adds_clicklabel.cpp \
    Core/dlg_add.cpp \
    Core/practice/dlg_practice.cpp \
    Media/database.cpp \
    Media/audioplayer.cpp \
    Core/practice/pract1.cpp

HEADERS += \
        Core/item_list.h \
    Core/icontray.h \
    Core/ui_item_list.h \
    Core/dlg_imageview.h \
    Core/dlg_vwr.h \
    Core/dlg_edititem.h \
    Core/dlg_topics.h \
    Core/welcome.h \
    Core/dlg_share.h \
    Core/dlg_edittpc.h \
    Core/dlg_translate.h \
    Core/mainwindow.h \
    Core/vars_statics.h \
    Core/vars_session.h \
    Classes/adds_clicklabel.h \
    Core/dlg_add.h \
    Core/practice/dlg_practice.h \
    Media/database.h \
    Media/audioplayer.h \
    Core/practice/pract1.h

FORMS += \
        Ui/item_list.ui \
    Ui/dlg_imageview.ui \
    Ui/dlg_vwr.ui \
    Ui/dlg_edititem.ui \
    Ui/dlg_topics.ui \
    Ui/welcome.ui \
    Ui/dlg_share.ui \
    Ui/dlg_edittpc.ui \
    Ui/dlg_translate.ui \
    Ui/mainwindow.ui \
    Ui/dlg_add.ui \
    Ui/dlg_practice.ui \
    Ui/pract1.ui

DISTFILES += \
    android/AndroidManifest.xml \
    android/gradle/wrapper/gradle-wrapper.jar \
    android/gradlew \
    android/res/values/libs.xml \
    android/build.gradle \
    android/gradle/wrapper/gradle-wrapper.properties \
    android/gradlew.bat \
    android/AndroidManifest.xml \
    android/gradle/wrapper/gradle-wrapper.jar \
    android/gradlew \
    android/res/values/libs.xml \
    android/build.gradle \
    android/gradle/wrapper/gradle-wrapper.properties \
    android/gradlew.bat \
    android/AndroidManifest.xml \
    android/gradle/wrapper/gradle-wrapper.jar \
    android/gradlew \
    android/res/values/libs.xml \
    android/build.gradle \
    android/gradle/wrapper/gradle-wrapper.properties \
    android/gradlew.bat \
    android/AndroidManifest.xml \
    android/gradle/wrapper/gradle-wrapper.jar \
    android/gradlew \
    android/res/values/libs.xml \
    android/build.gradle \
    android/gradle/wrapper/gradle-wrapper.properties \
    android/gradlew.bat

ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android


QMAKE_CXXFLAGS += -std=gnu++14

DESTDIR = build/target/
OBJECTS_DIR = build/obj/
MOC_DIR = build/moc/
UI_DIR = build/ui
RCC_DIR = build/rcc/
RESOURCES += res.qrc


#target.path = $${PREFIX}/bin
#desktop.path = $${PREFIX}/share/applications
#desktop.files += qwit.desktop
#icon.path = $${PREFIX}/share/icons
#icon.files += images/qwit.png
#INSTALLS = target desktop icon
