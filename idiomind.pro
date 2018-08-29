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
        src/main.cpp \
        src/icontray.cpp \
        src/item_list.cpp \
    src/dlg_imageview.cpp \
    src/dlg_vwr.cpp \
    src/dlg_edititem.cpp \
    src/dlg_topics.cpp \
    src/welcome.cpp \
    src/dlg_share.cpp \
    src/dlg_edittpc.cpp \
    src/dlg_translate.cpp \
    src/mainwindow.cpp \
    src/vars_statics.cpp \
    src/vars_session.cpp \
    src/adds_clicklabel.cpp \
    src/widgets/cmenucontex.cpp \
    src/dlg_add.cpp \
    src/dlg_practice.cpp

HEADERS += \
        src/item_list.h \
    src/icontray.h \
    src/ui_item_list.h \
    src/dlg_imageview.h \
    src/dlg_vwr.h \
    src/dlg_edititem.h \
    src/dlg_topics.h \
    src/welcome.h \
    src/dlg_share.h \
    src/dlg_edittpc.h \
    src/dlg_translate.h \
    src/mainwindow.h \
    src/vars_statics.h \
    src/vars_session.h \
    src/adds_clicklabel.h \
    src/widgets/cmenucontex.h \
    src/dlg_add.h \
    src/dlg_practice.h

FORMS += \
        src/item_list.ui \
    src/dlg_imageview.ui \
    src/dlg_vwr.ui \
    src/dlg_edititem.ui \
    src/dlg_topics.ui \
    src/welcome.ui \
    src/dlg_share.ui \
    src/dlg_edittpc.ui \
    src/dlg_translate.ui \
    src/mainwindow.ui \
    src/dlg_add.ui \
    src/dlg_practice.ui

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

RC_ICONS = idiomind.ico
RC_ICONS = idiomind.png

QMAKE_CXXFLAGS += -std=gnu++14


RESOURCES += \
    res.qrc

#target.path = $${PREFIX}/bin
#desktop.path = $${PREFIX}/share/applications
#desktop.files += qwit.desktop
#icon.path = $${PREFIX}/share/icons
#icon.files += images/qwit.png
#INSTALLS = target desktop icon
