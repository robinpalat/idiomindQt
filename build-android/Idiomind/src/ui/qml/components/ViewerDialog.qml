// ViewerDialog: vista completa de un Item (word/sentence), layout fullscreen.
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Idiomind 1.0

Rectangle {
    id: viewer
    color: "#fafafa"
    signal close()

    property var proxy: App.topicProxy
    property int itemIndex: 0
    property var itemData: ({})
    property real footerWideWidth: Math.min(80, Math.max(64, width * 0.20))
    property real footerActionWidth: Math.min(44, Math.max(36, width * 0.12))
    property real footerCloseWidth: Math.min(44, Math.max(36, width * 0.11))

    Component.onCompleted: loadItem()
    onItemIndexChanged: loadItem()

    function loadItem() {
        if (proxy) itemData = proxy.itemAt(itemIndex) || ({})
    }

    ColumnLayout {
        anchors.fill: parent; spacing: 0

        // === CONTENT AREA ===
        Flickable {
            id: contentFlickable
            Layout.fillWidth: true; Layout.fillHeight: true
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
                    y: Math.max(24, (parent.height - height) * 0.30)
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

                        // Words table: exactly two aligned columns.
                        ColumnLayout {
                            id: wordsTable
                            Layout.fillWidth: true; Layout.bottomMargin: 16
                            visible: itemData.wrds && itemData.wrds !== ""
                            property var wordParts: (itemData.wrds || "").split("_")
                            spacing: 0

                            Rectangle { Layout.fillWidth: true; height: 1; color: "#e8e8e8" }

                            RowLayout {
                                Layout.fillWidth: true; height: 36; spacing: 0
                                Rectangle {
                                    Layout.fillWidth: true; height: 36; color: "#f5f5f5"
                                    Text {
                                        anchors.fill: parent; anchors.leftMargin: 12
                                        text: "TARGET"; font.pixelSize: 11; font.bold: true; color: "#999"
                                        verticalAlignment: Text.AlignVCenter
                                    }
                                }
                                Rectangle { width: 1; height: 36; color: "#e8e8e8" }
                                Rectangle {
                                    Layout.fillWidth: true; height: 36; color: "#f5f5f5"
                                    Text {
                                        anchors.fill: parent; anchors.leftMargin: 12
                                        text: "SOURCE"; font.pixelSize: 11; font.bold: true; color: "#999"
                                        verticalAlignment: Text.AlignVCenter
                                    }
                                }
                            }

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

        // === FOOTER TOOLBAR ===
        Rectangle {
            Layout.fillWidth: true; height: 56; color: "#ffffff"
            border.color: "#e0e0e0"; border.width: 1

            RowLayout {
                anchors.fill: parent; anchors.leftMargin: 8; anchors.rightMargin: 8; spacing: 6

                // Close
                Rectangle {
                    width: viewer.footerCloseWidth; height: 36; radius: 6
                    color: closeMouse.pressed ? "#f0f0f0" : "#fafafa"; border.color: "#ddd"
                    Text { anchors.centerIn: parent; text: "\u2715"; font.pixelSize: 14; color: "#888" }
                    MouseArea { id: closeMouse; anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: viewer.close() }
                }

                // Previous
                Rectangle {
                    width: viewer.footerWideWidth; height: 36; radius: 6
                    color: prevMouse.pressed ? "#f0f0f0" : "#fafafa"; border.color: "#ddd"
                    visible: viewer.itemIndex > 0
                    Text { anchors.centerIn: parent; text: "\u25C0 Ant"; font.pixelSize: 12; color: "#555" }
                    MouseArea { id: prevMouse; anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: { viewer.itemIndex--; viewer.loadItem() } }
                }
                Item { visible: viewer.itemIndex <= 0; width: viewer.footerWideWidth }

                Item { Layout.fillWidth: true }

                // Audio
                Rectangle {
                    width: viewer.footerActionWidth; height: 36; radius: 6
                    color: audioMouse.pressed ? "#f0f0f0" : "#fafafa"; border.color: "#ddd"
                    Text { anchors.centerIn: parent; text: "\u25B6"; font.pixelSize: 14; color: "#555" }
                    MouseArea { id: audioMouse; anchors.fill: parent; cursorShape: Qt.PointingHandCursor }
                }

                // Edit
                Rectangle {
                    width: viewer.footerActionWidth; height: 36; radius: 6
                    color: editMouse.pressed ? "#f0f0f0" : "#fafafa"; border.color: "#ddd"
                    Text { anchors.centerIn: parent; text: "\u270E"; font.pixelSize: 14; color: "#555" }
                    MouseArea { id: editMouse; anchors.fill: parent; cursorShape: Qt.PointingHandCursor }
                }

                Item { Layout.fillWidth: true }

                // Next
                Rectangle {
                    width: viewer.footerWideWidth; height: 36; radius: 6
                    color: nextMouse.pressed ? "#f0f0f0" : "#fafafa"; border.color: "#ddd"
                    visible: proxy && viewer.itemIndex < proxy.itemCount() - 1
                    Text { anchors.centerIn: parent; text: "Sig \u25B6"; font.pixelSize: 12; color: "#555" }
                    MouseArea { id: nextMouse; anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: { viewer.itemIndex++; viewer.loadItem() } }
                }
            }
        }
    }
}
