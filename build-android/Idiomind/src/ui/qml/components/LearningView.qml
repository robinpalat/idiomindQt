// LearningView: datos reales + checkbox interactivo + doble clic → Viewer.
import QtQuick
import QtQuick.Layouts
import Idiomind 1.0

Rectangle {
    color: "#fafafa"

    property var proxy: App.topicProxy
    signal openViewer(int index)

    ListView {
        id: listView
        anchors.fill: parent; clip: true
        model: proxy ? proxy.indexModel : null
        currentIndex: -1

        Text {
            anchors.centerIn: parent
            text: proxy && proxy.activeTopic ? "No hay ítems en este topic" : "Selecciona un topic"
            font.pixelSize: 14; color: "#999"
            visible: proxy && proxy.indexModel && proxy.indexModel.itemCount === 0
        }

        delegate: Rectangle {
            width: parent ? parent.width : 0; height: 44
            color: ListView.isCurrentItem ? "#eef5ff" : (index % 2 === 0 ? "#ffffff" : "#fafafa")
            RowLayout {
                anchors.fill: parent; anchors.leftMargin: 16; anchors.rightMargin: 16
                Rectangle {
                    width: 20; height: 20; radius: 3
                    border.color: model.checked ? "#3b82f6" : "#ccc"; border.width: 2
                    color: model.checked ? "#3b82f6" : "transparent"
                    Text { anchors.centerIn: parent; text: model.checked ? "\u2713" : ""; font.pixelSize: 12; color: "white" }
                    MouseArea {
                        anchors.fill: parent; cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            listView.currentIndex = index
                            if (!model.checked && proxy) proxy.markItemAsLearned(model.plainText)
                        }
                    }
                }
                Item {
                    Layout.fillWidth: true; Layout.fillHeight: true
                    Column {
                        anchors.fill: parent
                        Text { text: model.plainText; font.pixelSize: 14; color: "#333" }
                        Text { text: model.srce; font.pixelSize: 11; color: "#999" }
                    }
                    MouseArea {
                        anchors.fill: parent
                        onClicked: listView.currentIndex = index
                        onDoubleClicked: {
                            listView.currentIndex = index
                            openViewer(index)
                        }
                    }
                }
            }

        }
    }
}
