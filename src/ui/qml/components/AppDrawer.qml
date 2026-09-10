// F6-A.1: Drawer lateral izquierdo — funciones globales.
import QtQuick
import QtQuick.Layouts
import Idiomind 1.0

Rectangle {
    id: drawer
    property bool open: false
    signal close()
    signal newTopic()
    signal importTopic()
    signal statistics()
    signal config()
    signal about()
    signal exit()

    width: parent ? parent.width * 0.75 : 300
    height: parent ? parent.height : 0
    x: open ? 0 : -width
    color: "#ffffff"
    z: 100

    Behavior on x { NumberAnimation { duration: 200; easing.type: Easing.InOutQuad } }

    ColumnLayout {
        anchors.fill: parent; spacing: 0

        // Header
        Rectangle {
            Layout.fillWidth: true; height: 56; color: "#3b82f6"
            Text {
                anchors.left: parent.left; anchors.leftMargin: 20
                anchors.verticalCenter: parent.verticalCenter
                text: App.name()
                font.pixelSize: 18; font.bold: true; color: "#ffffff"
            }
        }

        Rectangle { Layout.fillWidth: true; height: 1; color: "#e0e0e0" }

        // Menu items
        Repeater {
            model: ListModel {
                ListElement { label: "Nuevo Topic";    icon: "+" }
                ListElement { label: "Abrir Topic";    icon: "\u2191" }
                ListElement { label: "Estadísticas";   icon: "#" }
                ListElement { label: "Configuración";  icon: "*" }
                ListElement { label: "Acerca de...";   icon: "?" }
                ListElement { label: "Salir";          icon: "x" }
            }
            delegate: Rectangle {
                Layout.fillWidth: true; height: 48
                color: itemMouse.pressed ? "#f0f4ff" : "transparent"

                RowLayout {
                    anchors.fill: parent; anchors.leftMargin: 20; anchors.rightMargin: 20
                    Text { text: model.icon; font.pixelSize: 16; color: "#888"; width: 24 }
                    Text { text: model.label; font.pixelSize: 14; color: "#333"; Layout.fillWidth: true }
                }

                MouseArea {
                    id: itemMouse; anchors.fill: parent
                    onClicked: {
                        drawer.close()
                        if (index === 0) drawer.newTopic()
                        else if (index === 1) drawer.importTopic()
                        else if (index === 2) drawer.statistics()
                        else if (index === 3) drawer.config()
                        else if (index === 4) drawer.about()
                        else if (index === 5) drawer.exit()
                    }
                }
            }
        }

        Item { Layout.fillHeight: true }

        // Version
        Text {
            Layout.alignment: Qt.AlignHCenter
            text: "v" + App.version()
            font.pixelSize: 10; color: "#bbb"
        }

        Item { height: 12 }
    }

    // Backdrop
    Rectangle {
        anchors.left: drawer.right; anchors.right: parent.right; anchors.top: parent.top; anchors.bottom: parent.bottom
        color: "#00000040"
        visible: drawer.open
        MouseArea { anchors.fill: parent; onClicked: drawer.close() }
    }
}
