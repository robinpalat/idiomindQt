// F6-C.4: PracticeDialog — lista + resultado, footer inferior.
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Idiomind 1.0

Rectangle {
    id: dialog
    color: "#ffffff"
    signal close()

    property var proxy: App.topicProxy
    property int state: 0  // 0=PRACTICE_LIST, 1=PRACTICE_RESULT

    // Result data (set when practice completes)
    property string resultTitle: ""
    property string resultInfo: ""
    property int resultImageIndex: 0

    ColumnLayout {
        anchors.fill: parent; spacing: 0

        // --- Content area ---
        StackLayout {
            Layout.fillWidth: true; Layout.fillHeight: true
            currentIndex: dialog.state

            // PRACTICE_LIST
            Item {
                ListView {
                    anchors.fill: parent; clip: true
                    model: ListModel {
                        ListElement { name: "Tarjetas de vocabulario"; desc: "Flashcards con traducción"; imgFile: "qrc:/images/practice/10.png" }
                        ListElement { name: "Multiple choice"; desc: "Selecciona la respuesta correcta"; imgFile: "qrc:/images/practice/5.png" }
                        ListElement { name: "Reconocer pronunciación"; desc: "Escucha y escribe"; imgFile: "qrc:/images/practice/15.png" }
                        ListElement { name: "Imágenes"; desc: "Asocia imagen con palabra"; imgFile: "qrc:/images/practice/21.png" }
                        ListElement { name: "Escuchar y escribir"; desc: "Dictado de oraciones"; imgFile: "qrc:/images/practice/0.png" }
                    }
                    delegate: Item {
                        width: parent ? parent.width : 0; height: 72
                        RowLayout {
                            anchors.fill: parent; anchors.leftMargin: 16; anchors.rightMargin: 16
                            spacing: 16
                            Image {
                                width: 64; height: 64
                                source: model.imgFile
                                fillMode: Image.PreserveAspectFit
                                sourceSize: Qt.size(64, 64)
                                asynchronous: true
                            }
                            Column {
                                Layout.fillWidth: true; spacing: 4
                                Text { text: model.name; font.pixelSize: 16; font.bold: true; color: "#333" }
                                Text { text: model.desc; font.pixelSize: 13; color: "#888" }
                            }
                        }
                        MouseArea {
                            anchors.fill: parent
                            cursorShape: Qt.PointingHandCursor
                            onClicked: {
                                dialog.state = 1
                                dialog.resultTitle = model.name
                                dialog.resultInfo = "Práctica completada"
                                dialog.resultImageIndex = 21
                            }
                        }
                    }
                }
            }

            // PRACTICE_RESULT
            Item {
                ColumnLayout {
                    anchors.fill: parent; anchors.margins: 24; spacing: 16

                    Item { Layout.fillHeight: true }

                    // Result image
                    Image {
                        Layout.alignment: Qt.AlignHCenter
                        width: 128; height: 128
                        source: "qrc:/images/practice/" + dialog.resultImageIndex + ".png"
                        fillMode: Image.PreserveAspectFit
                        sourceSize: Qt.size(128, 128)
                    }

                    // Result title
                    Text {
                        Layout.alignment: Qt.AlignHCenter
                        text: dialog.resultTitle
                        font.pixelSize: 18; font.bold: true; color: "#333"
                    }

                    // Result info
                    Text {
                        Layout.alignment: Qt.AlignHCenter
                        text: dialog.resultInfo
                        font.pixelSize: 14; color: "#666"
                        horizontalAlignment: Text.AlignHCenter
                    }

                    Item { Layout.fillHeight: true }
                }
            }
        }

        // --- Footer toolbar ---
        Rectangle {
            Layout.fillWidth: true; height: 56
            color: "#f5f5f5"
            border.color: "#e0e0e0"; border.width: 1

            RowLayout {
                anchors.fill: parent; anchors.leftMargin: 16; anchors.rightMargin: 16

                // REINICIAR
                Rectangle {
                    width: 120; height: 36; radius: 6
                    color: restartMouse.pressed ? "#e0e0e0" : "#f5f5f5"
                    border.color: "#ccc"
                    Text { anchors.centerIn: parent; text: "REINICIAR"; font.pixelSize: 12; font.bold: true; color: "#555" }
                    MouseArea {
                        id: restartMouse; anchors.fill: parent; cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            if (proxy) proxy.restartPractice()
                            dialog.state = 0
                        }
                    }
                }

                Item { Layout.fillWidth: true }

                // CERRAR
                Rectangle {
                    width: 120; height: 36; radius: 6
                    color: closeMouse.pressed ? "#e0e0e0" : "#f5f5f5"
                    border.color: "#ccc"
                    Text { anchors.centerIn: parent; text: "CERRAR"; font.pixelSize: 12; font.bold: true; color: "#555" }
                    MouseArea {
                        id: closeMouse; anchors.fill: parent; cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            if (dialog.state === 1) {
                                dialog.state = 0  // Volver a lista
                            } else {
                                dialog.close()    // Cerrar diálogo
                            }
                        }
                    }
                }
            }
        }
    }
}
