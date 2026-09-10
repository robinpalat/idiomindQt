// F6-C.5: Vista "Administrar" — ficha informativa del topic.
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Idiomind 1.0

Rectangle {
    color: "#fafafa"
    signal openTopics()

    property var proxy: App.topicProxy

    Flickable {
        anchors.fill: parent; clip: true
        contentWidth: width
        contentHeight: contentCol.height

        ColumnLayout {
            id: contentCol
            width: parent.width - 40
            x: 20; spacing: 0

            // Empty state
            Text {
                Layout.fillWidth: true; Layout.alignment: Qt.AlignHCenter
                text: "Selecciona un topic para ver su información"
                font.pixelSize: 14; color: "#999"
                visible: proxy && (!proxy.activeTopic || proxy.activeTopic === "")
                Layout.topMargin: 40
            }

            // Topic title (large, bold, no prefix)
            Text {
                Layout.fillWidth: true
                text: proxy ? proxy.activeTopic : ""
                font.pixelSize: 22; font.bold: true; color: "#222"
                visible: proxy && proxy.activeTopic !== ""
                wrapMode: Text.Wrap
            }

            // Info metadata (compact, no border)
            ColumnLayout {
                Layout.fillWidth: true; spacing: 4
                visible: proxy && proxy.activeTopic !== ""
                Layout.topMargin: 12

                Repeater {
                    model: ListModel {
                        ListElement { label: "Estado"; field: "stts" }
                        ListElement { label: "Palabras"; field: "nwrd" }
                        ListElement { label: "Oraciones"; field: "nsnt" }
                        ListElement { label: "Imágenes"; field: "nimg" }
                        ListElement { label: "Creado"; field: "dtec" }
                        ListElement { label: "Autor"; field: "autr" }
                        ListElement { label: "Idioma"; field: "idioma" }
                        ListElement { label: "Categoría"; field: "ctgy" }
                    }
                    delegate: RowLayout {
                        Layout.fillWidth: true
                        spacing: 8
                        visible: {
                            if (!proxy) return false
                            if (model.field === "idioma")
                                return proxy.slng !== ""
                            var val = proxy[model.field]
                            return val !== undefined && val !== ""
                        }
                        Text {
                            text: model.label
                            font.pixelSize: 12; color: "#888"
                            Layout.preferredWidth: 90
                        }
                        Text {
                            text: {
                                if (!proxy) return ""
                                if (model.field === "idioma")
                                    return proxy.slng + " → " + proxy.tlng
                                return proxy[model.field]
                            }
                            font.pixelSize: 13; color: "#333"
                            Layout.fillWidth: true
                        }
                    }
                }
            }

            // Space between info and main action
            Item { Layout.fillWidth: true; height: 24
                visible: proxy && proxy.activeTopic !== ""
            }

            // Main action button (full width)
            Rectangle {
                Layout.fillWidth: true; height: 48; radius: 8
                color: mainActionMouse.pressed ? "#e8f5e9" : "#f1f8e9"
                border.color: proxy && proxy.actionLabel !== "" ? "#8bc34a" : "#e0e0e0"
                visible: proxy && proxy.activeTopic !== "" && proxy.actionLabel !== ""

                RowLayout {
                    anchors.fill: parent; anchors.leftMargin: 16; anchors.rightMargin: 16
                    spacing: 8
                    Text {
                        text: proxy.actionLabel.indexOf("REVISAR") >= 0 ? "\u2713" : "\u21BB"
                        font.pixelSize: 16; color: "#4caf50"
                    }
                    Text {
                        text: proxy ? proxy.actionLabel : ""
                        font.pixelSize: 14; font.bold: true; color: "#2e7d32"
                        Layout.fillWidth: true
                    }
                }
                MouseArea {
                    id: mainActionMouse; anchors.fill: parent; cursorShape: Qt.PointingHandCursor
                    onClicked: { if (proxy) proxy.performAction() }
                }
            }

            // Space between main action and secondary actions
            Item { Layout.fillWidth: true; height: 20
                visible: proxy && proxy.activeTopic !== ""
            }

            // Secondary actions row
            RowLayout {
                Layout.fillWidth: true; spacing: 8
                visible: proxy && proxy.activeTopic !== ""

                Repeater {
                    model: ListModel {
                        ListElement { label: "EDITAR"; icon: "\u270E" }
                        ListElement { label: "COMPARTIR"; icon: "\u2197" }
                        ListElement { label: "BORRAR"; icon: "\u2716" }
                    }
                    delegate: Rectangle {
                        Layout.fillWidth: true; height: 40; radius: 6
                        color: model.label === "BORRAR" ? "#fff5f5" : "#f5f5f5"
                        border.color: model.label === "BORRAR" ? "#ffcdd2" : "#e0e0e0"
                        RowLayout {
                            anchors.fill: parent; anchors.leftMargin: 8; anchors.rightMargin: 8
                            spacing: 6
                            Text { text: model.icon; font.pixelSize: 13; color: model.label === "BORRAR" ? "#e57373" : "#888" }
                            Text { text: model.label; font.pixelSize: 11; color: model.label === "BORRAR" ? "#c62828" : "#555"; font.bold: true }
                        }
                        MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor }
                    }
                }
            }

            Item { Layout.fillHeight: true }
        }
    }
}
