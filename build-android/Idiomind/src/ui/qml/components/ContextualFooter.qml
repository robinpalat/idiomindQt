// Footer contextual — solo acciones según pestaña, sin nombre.
import QtQuick
import QtQuick.Layouts
import Idiomind 1.0

Rectangle {
    id: footer
    property int currentTab: 0
    property var proxy: App.topicProxy
    signal play()
    signal practice()
    signal openTopics()

    height: 48; color: "#ffffff"
    border.color: "#e0e0e0"; border.width: 1
    visible: currentTab === 0 || currentTab === 3

    RowLayout {
        anchors.fill: parent; spacing: 0

        // Left action
        Item {
            Layout.fillWidth: true; Layout.fillHeight: true
            visible: currentTab === 0 || currentTab === 3

            Text {
                anchors.centerIn: parent
                text: currentTab === 0 ? "REPRODUCIR" : "TOPICS"
                font.pixelSize: 11; font.bold: true
                color: "#3b82f6"
            }
            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if (currentTab === 0) footer.play()
                    else footer.openTopics()
                }
            }
        }

        // Right action (Aprendiendo: PRACTICAR)
        Item {
            Layout.fillWidth: true; Layout.fillHeight: true
            visible: currentTab === 0

            Text {
                anchors.centerIn: parent
                text: "PRACTICAR"
                font.pixelSize: 11; font.bold: true
                color: "#3b82f6"
            }
            MouseArea {
                anchors.fill: parent
                onClicked: footer.practice()
            }
        }
    }
}
