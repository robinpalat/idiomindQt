// LearningView: notas target + checkbox interactivo + apertura por clic.
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Idiomind 1.0

Rectangle {
    color: "#fafafa"

    property var proxy: App.topicProxy
    property int noteCount: listView.count
    signal openViewer(int index)

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        Rectangle {
            Layout.fillWidth: true; Layout.fillHeight: true
            visible: proxy && proxy.reviewPanelVisible
            color: "#fafafa"

            ColumnLayout {
                anchors.left: parent.left; anchors.right: parent.right
                anchors.top: parent.top
                anchors.margins: 18
                spacing: 8
                Text {
                    text: "Topic aprendido"
                    font.pixelSize: 20; font.bold: true; color: "#26323d"
                }
                Text {
                    Layout.fillWidth: true
                    text: proxy ? proxy.reviewLabel : ""
                    font.pixelSize: 13; color: "#586673"
                }
                RowLayout {
                    Layout.fillWidth: true; spacing: 18
                    Text {
                        text: proxy ? "Waiting Days: " + proxy.reviewDays : ""
                        font.pixelSize: 13; color: "#586673"
                    }
                    Text {
                        Layout.fillWidth: true
                        text: proxy && proxy.reviewReady ? "Ready" : "Wait"
                        font.pixelSize: 13; font.bold: true
                        color: proxy && proxy.reviewReady ? "#347653" : "#8a6d3b"
                    }
                }
                Text {
                    Layout.fillWidth: true
                    text: "Spacing Intervals for Review: " + (proxy ? proxy.reviewIntervalsLabel : "")
                    textFormat: Text.RichText
                    font.pixelSize: 13; color: "#586673"
                }
                ProgressBar {
                    Layout.fillWidth: true
                    from: 0; to: 100
                    value: proxy ? Math.max(0, Math.min(100, proxy.reviewPercent)) : 0
                }
            }
        }

        ListView {
            id: listView
            Layout.fillWidth: true; Layout.fillHeight: true
            clip: true
            visible: !proxy || !proxy.reviewPanelVisible
            model: proxy ? proxy.indexModel : null
            currentIndex: -1

            Text {
                anchors.centerIn: parent
                text: proxy && proxy.activeTopic ? "No hay ítems en este topic" : "Selecciona un topic"
                font.pixelSize: 14; color: "#999"
                visible: proxy && proxy.indexModel && proxy.indexModel.itemCount === 0
            }

            delegate: Rectangle {
                width: parent ? parent.width : 0
                height: Math.max(44, targetText.implicitHeight + 16)
                color: ListView.isCurrentItem ? "#eef5ff" : (index % 2 === 0 ? "#ffffff" : "#fafafa")
                RowLayout {
                    anchors.fill: parent; anchors.leftMargin: 16; anchors.rightMargin: 16
                    Item {
                        Layout.fillWidth: true; Layout.fillHeight: true
                        Column {
                            anchors.left: parent.left; anchors.right: parent.right
                            anchors.verticalCenter: parent.verticalCenter
                            Text {
                                id: targetText
                                width: parent.width
                                text: model.plainText
                                font.pixelSize: 14; color: "#333"
                                wrapMode: Text.Wrap
                                maximumLineCount: 2
                                elide: Text.ElideRight
                            }
                        }
                        MouseArea {
                            anchors.fill: parent
                            onClicked: {
                                listView.currentIndex = index
                                openViewer(index)
                            }
                        }
                    }
                    Rectangle {
                        Layout.alignment: Qt.AlignVCenter
                        width: 20; height: 20; radius: 3
                        border.color: model.checked ? "#3b82f6" : "#ccc"; border.width: 2
                        color: model.checked ? "#3b82f6" : "transparent"
                        Text { anchors.centerIn: parent; text: model.checked ? "\u2713" : ""; font.pixelSize: 12; color: "white" }
                        MouseArea {
                            anchors.fill: parent; cursorShape: Qt.PointingHandCursor
                            onClicked: { if (!model.checked && proxy) proxy.markItemAsLearned(model.plainText) }
                        }
                    }
                }
            }
        }
    }
}
