// ViewerDialog: vista completa de un Item (word/sentence), layout fullscreen.
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtMultimedia
import Idiomind 1.0

Rectangle {
    id: viewer
    color: "#fafafa"
    signal close()

    property var proxy: App.topicProxy
    property int itemIndex: 0
    property int activeIndex: 0
    property var itemData: ({})

    function audioUrl(path) {
        if (!path || path === "")
            return ""
        return path.indexOf("://") >= 0 ? path : "file://" + path
    }

    MediaPlayer {
        id: audioPlayer
        audioOutput: AudioOutput {}
        source: itemData && itemData.audioSource ? audioUrl(itemData.audioSource) : ""
    }

    Component.onCompleted: {
        activeIndex = itemIndex
        loadItem()
    }
    onItemIndexChanged: {
        activeIndex = itemIndex
        loadItem()
    }
    onVisibleChanged: {
        if (visible) {
            activeIndex = itemIndex
            loadItem()
        }
    }

    function loadItem() {
        if (proxy) itemData = proxy.itemAt(activeIndex) || ({})
    }

    // === CONTENT ===
    Flickable {
        id: contentFlickable
        anchors.top: parent.top
        anchors.topMargin: 60
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 56
        contentWidth: width
        contentHeight: contentCanvas.height
        clip: true
        boundsBehavior: Flickable.StopAtBounds

        Item {
            id: contentCanvas
            width: contentFlickable.width
            height: Math.max(contentFlickable.height, contentBlock.implicitHeight + 48)

            ColumnLayout {
                id: contentBlock
                width: Math.max(0, Math.min(parent.width - 48, 720))
                height: implicitHeight
                x: (parent.width - width) / 2
                    y: itemData.type === "1"
                       ? Math.max(24, (parent.height - height) * 0.30)
                       : 24
                spacing: 0

                // --- WORD VIEW (type == "1") ---
                ColumnLayout {
                    Layout.fillWidth: true
                    visible: itemData.type === "1"
                    spacing: 0

                    Text {
                        Layout.fillWidth: true; Layout.bottomMargin: 6
                        text: itemData.trgt || ""
                        font.pixelSize: 28; font.bold: true; color: "#222"
                        horizontalAlignment: Text.AlignHCenter
                        wrapMode: Text.Wrap
                    }

                    Text {
                        Layout.fillWidth: true; Layout.bottomMargin: 24
                        text: itemData.srce || ""
                        font.pixelSize: 18; color: "#555"
                        horizontalAlignment: Text.AlignHCenter
                        wrapMode: Text.Wrap
                         font.italic: true
                    }

                    Text {
                        Layout.fillWidth: true; Layout.bottomMargin: 16
                        visible: itemData.link && itemData.link !== ""
                        text: itemData.link ? ("<a href='" + itemData.link + "'>" + itemData.link + "</a>") : ""
                        font.pixelSize: 13; color: "#3b82f6"
                        horizontalAlignment: Text.AlignHCenter
                        textFormat: Text.RichText
                        onLinkActivated: Qt.openUrlExternally(link)
                    }

                    ColumnLayout {
                        Layout.fillWidth: true; Layout.bottomMargin: 16
                        visible: itemData.exmp && itemData.exmp !== ""
                        spacing: 6
                        Text {
                            Layout.fillWidth: true
                            text: "Ejemplo"
                            font.pixelSize: 11; font.bold: true; color: "#aaa"
                            horizontalAlignment: Text.AlignHCenter
                        }
                        Text {
                            Layout.fillWidth: true
                            text: itemData.exmp || ""
                            font.pixelSize: 14; font.italic: true; color: "#444"
                            horizontalAlignment: Text.AlignHCenter
                            wrapMode: Text.Wrap
                        }
                    }

                    ColumnLayout {
                        Layout.fillWidth: true; Layout.bottomMargin: 16
                        visible: itemData.defn && itemData.defn !== ""
                        spacing: 6
                        Text {
                            Layout.fillWidth: true
                            text: "Definición"
                            font.pixelSize: 11; font.bold: true; color: "#aaa"
                            horizontalAlignment: Text.AlignHCenter
                        }
                        Text {
                            Layout.fillWidth: true
                            text: itemData.defn || ""
                            font.pixelSize: 14; color: "#555"
                            horizontalAlignment: Text.AlignHCenter
                            wrapMode: Text.Wrap
                        }
                    }

                    Text {
                        Layout.fillWidth: true; Layout.bottomMargin: 16
                        visible: itemData.tags && itemData.tags !== ""
                        text: itemData.tags || ""
                        font.pixelSize: 12; color: "#999"
                        horizontalAlignment: Text.AlignHCenter
                    }

                    ColumnLayout {
                        Layout.fillWidth: true; Layout.bottomMargin: 16
                        visible: itemData.note && itemData.note !== ""
                        spacing: 6
                        Text {
                            Layout.fillWidth: true
                            text: "Nota"
                            font.pixelSize: 11; font.bold: true; color: "#aaa"
                            horizontalAlignment: Text.AlignHCenter
                        }
                        Text {
                            Layout.fillWidth: true
                            text: itemData.note || ""
                            font.pixelSize: 13; color: "#666"
                            horizontalAlignment: Text.AlignHCenter
                            wrapMode: Text.Wrap
                        }
                    }
                }

                // --- SENTENCE VIEW (type != "1") ---
                ColumnLayout {
                    Layout.fillWidth: true
                    visible: itemData.type !== "1"
                    spacing: 0

                    Text {
                        Layout.fillWidth: true; Layout.bottomMargin: 6
                        text: itemData.grmr && itemData.grmr !== "" ? itemData.grmr : (itemData.trgt || "")
                        font.pixelSize: 22; color: "#222"
                        horizontalAlignment: Text.AlignHCenter
                        wrapMode: Text.Wrap
                        textFormat: Text.RichText
                    }

                    Text {
                        Layout.fillWidth: true; Layout.bottomMargin: 24
                        text: itemData.srce || ""
                        font.pixelSize: 16; color: "#555"
                        horizontalAlignment: Text.AlignHCenter
                        wrapMode: Text.Wrap
                        font.italic: true
                    }

                    Text {
                        Layout.fillWidth: true; Layout.bottomMargin: 16
                        visible: itemData.link && itemData.link !== ""
                        text: itemData.link ? ("<a href='" + itemData.link + "'>" + itemData.link + "</a>") : ""
                        font.pixelSize: 13; color: "#3b82f6"
                        horizontalAlignment: Text.AlignHCenter
                        textFormat: Text.RichText
                        onLinkActivated: Qt.openUrlExternally(link)
                    }

                    ColumnLayout {
                        id: wordsTable
                        Layout.fillWidth: true; Layout.bottomMargin: 16
                        visible: itemData.wrds && itemData.wrds !== ""
                        property var wordParts: (itemData.wrds || "").split("_")
                        spacing: 0

                        Rectangle { Layout.fillWidth: true; height: 1; color: "#e8e8e8" }

                        // RowLayout {
                        //     Layout.fillWidth: true; height: 36; spacing: 0
                        //     Rectangle {
                        //         Layout.fillWidth: true; height: 36; color: "#f5f5f5"
                        //         Text {
                        //             anchors.fill: parent; anchors.leftMargin: 12
                        //             text: "TARGET"; font.pixelSize: 11; font.bold: true; color: "#999"
                        //             verticalAlignment: Text.AlignVCenter
                        //         }
                        //     }
                        //     Rectangle { width: 1; height: 36; color: "#e8e8e8" }
                        //     Rectangle {
                        //         Layout.fillWidth: true; height: 36; color: "#f5f5f5"
                        //         Text {
                        //             anchors.fill: parent; anchors.leftMargin: 12
                        //             text: "SOURCE"; font.pixelSize: 11; font.bold: true; color: "#999"
                        //             verticalAlignment: Text.AlignVCenter
                        //         }
                        //     }
                        // }

                        Rectangle { Layout.fillWidth: true; height: 1; color: "#e8e8e8" }

                        Repeater {
                            model: Math.floor(wordsTable.wordParts.length / 2)
                            delegate: RowLayout {
                                Layout.fillWidth: true; height: 36; spacing: 0
                                Rectangle {
                                    Layout.fillWidth: true; height: 36
                                    color: index % 2 === 0 ? "#ffffff" : "#fafafa"
                                    Text {
                                        anchors.fill: parent; anchors.leftMargin: 12
                                        text: wordsTable.wordParts[index * 2] || ""
                                        font.pixelSize: 14; color: "#333"
                                        verticalAlignment: Text.AlignVCenter
                                    }
                                }
                                Rectangle { width: 1; height: 36; color: "#e8e8e8" }
                                Rectangle {
                                    Layout.fillWidth: true; height: 36
                                    color: index % 2 === 0 ? "#ffffff" : "#fafafa"
                                    Text {
                                        anchors.fill: parent; anchors.leftMargin: 12
                                        text: wordsTable.wordParts[index * 2 + 1] || ""
                                        font.pixelSize: 14; color: "#666"
                                        verticalAlignment: Text.AlignVCenter
                                        font.italic: true
                                    }
                                }
                            }
                        }

                        Rectangle { Layout.fillWidth: true; height: 1; color: "#e8e8e8" }
                    }

                    Text {
                        Layout.fillWidth: true; Layout.bottomMargin: 16
                        visible: itemData.tags && itemData.tags !== ""
                        text: itemData.tags || ""
                        font.pixelSize: 12; color: "#999"
                        horizontalAlignment: Text.AlignHCenter
                    }

                    ColumnLayout {
                        Layout.fillWidth: true; Layout.bottomMargin: 16
                        visible: itemData.note && itemData.note !== ""
                        spacing: 6
                        Text {
                            Layout.fillWidth: true
                            text: "Nota"
                            font.pixelSize: 11; font.bold: true; color: "#aaa"
                            horizontalAlignment: Text.AlignHCenter
                        }
                        Text {
                            Layout.fillWidth: true
                            text: itemData.note || ""
                            font.pixelSize: 13; color: "#666"
                            horizontalAlignment: Text.AlignHCenter
                            wrapMode: Text.Wrap
                        }
                    }
                }
            }
        }
    }

    // === TOP-RIGHT CONTROLS (floating over content) ===

    // Header
    Item {
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        height: 60

        // Botón Editar
        Item {
            anchors.left: parent.left
            anchors.leftMargin: 12
            anchors.verticalCenter: parent.verticalCenter
            width: 52
            height: 52

            Rectangle {
                anchors.fill: parent
                radius: 8
                color: editTop.pressed ? "#e8ecef" : "transparent"

                Text {
                    anchors.centerIn: parent
                    text: ""
                    font.pixelSize: 20
                    color: "#555"
                }
            }

            MouseArea {
                id: editTop
                anchors.fill: parent
                cursorShape: Qt.PointingHandCursor

                onClicked: {
                    // conservar aquí la acción actual de editar
                }
            }
        }

        // Botón Cerrar
        Item {
            anchors.right: parent.right
            anchors.rightMargin: 12
            anchors.verticalCenter: parent.verticalCenter
            width: 52
            height: 52

            Rectangle {
                anchors.fill: parent
                radius: 8
                color: closeTop.pressed ? "#e8ecef" : "transparent"

                Text {
                    anchors.centerIn: parent
                    text: "×"
                    font.pixelSize: 28
                    font.weight: Font.Light
                    color: "#555"
                }
            }

            MouseArea {
                id: closeTop
                anchors.fill: parent
                cursorShape: Qt.PointingHandCursor

                onClicked: viewer.close()
            }
        }
    }

    // === BOTTOM NAVIGATION ===
    RowLayout {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 8
        anchors.leftMargin: 12
        anchors.rightMargin: 12

        height: 52
        spacing: 34

        // Previous
        Item {
            width: 52
            height: 52
            visible: viewer.activeIndex > 0

            Rectangle {
                anchors.fill: parent
                radius: 8
                color: prevNav.pressed ? "#e8ecef" : "transparent"

                Text {
                    anchors.centerIn: parent
                    text: "‹"
                    font.pixelSize: 34
                    font.weight: Font.Light
                    color: "#222"
                }
            }

            MouseArea {
                id: prevNav
                anchors.fill: parent
                cursorShape: Qt.PointingHandCursor

                onClicked: {
                    viewer.activeIndex--
                    viewer.loadItem()
                }
            }
        }

        // Si no existe anterior, conserva el espacio
        Item {
            width: 52
            height: 52
            visible: viewer.activeIndex <= 0
        }

        Item {
            Layout.fillWidth: true
        }

        // Audio
        Item {
            width: 52
            height: 52

            Rectangle {
                anchors.fill: parent
                radius: 8
                color: audioNav.pressed ? "#e8ecef" : "transparent"

                Text {
                    anchors.centerIn: parent
                    text: "▶"
                    font.pixelSize: 18
                    color: "#555"
                }
            }

            MouseArea {
                id: audioNav
                anchors.fill: parent
                cursorShape: Qt.PointingHandCursor

                onClicked: audioPlayer.play()
            }
        }

        // Next
        Item {
            width: 52
            height: 52
            visible: proxy && viewer.activeIndex < proxy.itemCount() - 1

            Rectangle {
                anchors.fill: parent
                radius: 8
                color: nextNav.pressed ? "#e8ecef" : "transparent"

                Text {
                    anchors.centerIn: parent
                    text: "›"
                    font.pixelSize: 34
                    font.weight: Font.Light
                    color: "#222"
                }
            }

            MouseArea {
                id: nextNav
                anchors.fill: parent
                cursorShape: Qt.PointingHandCursor

                onClicked: {
                    viewer.activeIndex++
                    viewer.loadItem()
                }
            }
        }
    }
}
