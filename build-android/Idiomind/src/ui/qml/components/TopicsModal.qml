// F6-C.7: Modal de Topics — pantalla completa, toolbar estilo PracticeDialog.
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Idiomind 1.0

Rectangle {
    id: modal
    color: "#ffffff"
    signal close()
    signal importTopic()

    property var proxy: App.topicProxy

    Component.onCompleted: { if (proxy) proxy.refreshTopics() }

    ColumnLayout {
        anchors.fill: parent; spacing: 0

        // Empty state
        Text {
            Layout.fillWidth: true; Layout.alignment: Qt.AlignHCenter
            text: "No hay topics en este idioma"
            font.pixelSize: 14; color: "#999"
            visible: proxy ? proxy.topicCount === 0 : true
            Layout.topMargin: 40
        }

        // Topic list
        ListView {
            Layout.fillWidth: true; Layout.fillHeight: true; clip: true
            model: proxy ? proxy.topicModel : null
            delegate: Item {
                width: parent ? parent.width : 0; height: 60
                RowLayout {
                    anchors.fill: parent; anchors.leftMargin: 16; anchors.rightMargin: 16; spacing: 12
                    Image {
                        width: 40; height: 40
                        source: proxy ? proxy.stateImage(model.stts) : ""
                        fillMode: Image.PreserveAspectFit
                        sourceSize: Qt.size(40, 40)
                    }
                    Text { Layout.fillWidth: true; text: model.name; font.pixelSize: 16; font.bold: true; color: "#333"; elide: Text.ElideRight }
                }
                MouseArea { anchors.fill: parent; onClicked: { if (proxy) proxy.selectTopic(model.name); modal.close() } }
            }
        }

        // Footer toolbar (same style as PracticeDialog)
        Rectangle {
            Layout.fillWidth: true; height: 56; color: "#f5f5f5"
            border.color: "#e0e0e0"; border.width: 1

            RowLayout {
                anchors.fill: parent; anchors.leftMargin: 16; anchors.rightMargin: 16

                Rectangle {
                    width: 100; height: 36; radius: 6; color: nuevoMouse.pressed ? "#e0e0e0" : "#f5f5f5"; border.color: "#ccc"
                    Text { anchors.centerIn: parent; text: "NUEVO"; font.pixelSize: 12; font.bold: true; color: "#555" }
                    MouseArea { id: nuevoMouse; anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: {} }
                }

                Item { Layout.fillWidth: true }

                Rectangle {
                    width: 100; height: 36; radius: 6; color: applyMouse.pressed ? "#e0e0e0" : "#f5f5f5"; border.color: "#ccc"
                    Text { anchors.centerIn: parent; text: "ABRIR"; font.pixelSize: 12; font.bold: true; color: "#555" }
                    MouseArea { id: applyMouse; anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: { modal.close(); modal.importTopic() } }
                }

                Rectangle {
                    width: 100; height: 36; radius: 6; color: closeMouse.pressed ? "#e0e0e0" : "#f5f5f5"; border.color: "#ccc"
                    Text { anchors.centerIn: parent; text: "CERRAR"; font.pixelSize: 12; font.bold: true; color: "#555" }
                    MouseArea { id: closeMouse; anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: modal.close() }
                }
            }
        }
    }
}
