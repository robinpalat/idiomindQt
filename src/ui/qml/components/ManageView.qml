// F6-C.5: Vista "Administrar" — ficha del topic + lista de topics.
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Idiomind 1.0

Rectangle {
    color: "#fafafa"
    signal openTopics()

    property var proxy: App.topicProxy

    Component.onCompleted: { if (proxy) proxy.refreshTopics() }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        // === TOPIC ACTIVE INFORMATION ===
        Flickable {
            Layout.fillWidth: true
            Layout.preferredHeight: infoColumn.implicitHeight + 20
            clip: true
            contentWidth: width
            contentHeight: infoColumn.implicitHeight

            ColumnLayout {
                id: infoColumn
                width: parent.width
                spacing: 0

                Text {
                    Layout.fillWidth: true
                    Layout.leftMargin: 16; Layout.rightMargin: 16
                    Layout.topMargin: 14; Layout.bottomMargin: 8
                    text: "Selecciona un topic para ver su información"
                    font.pixelSize: 14; color: "#7c8790"
                    horizontalAlignment: Text.AlignHCenter
                    visible: !proxy || !proxy.activeTopic || proxy.activeTopic === ""
                }

                ColumnLayout {
                    Layout.fillWidth: true
                    Layout.leftMargin: 16; Layout.rightMargin: 16
                    Layout.topMargin: 12; Layout.bottomMargin: 6
                    spacing: 2
                    visible: proxy && proxy.activeTopic !== ""
                    Text {
                        Layout.fillWidth: true
                        text: proxy ? proxy.activeTopic : ""
                        font.pixelSize: 20; font.bold: true; color: "#26323d"
                        elide: Text.ElideRight
                    }
                    Text {
                        Layout.fillWidth: true
                        text: proxy ? proxy.levelLabel : ""
                        font.pixelSize: 13; font.bold: true
                        color: proxy && proxy.levelLabelColor ? proxy.levelLabelColor : "#7c8790"
                        font.italic: true
                    }
                    Text {
                        Layout.fillWidth: true; Layout.topMargin: 4
                        text: proxy
                              ? "Palabras " + (proxy.nwrd || "0")
                                + "  ·  Oraciones " + (proxy.nsnt || "0")
                                + "  ·  Imágenes " + (proxy.nimg || "0")
                              : ""
                        font.pixelSize: 10; color: "#4e5b66"
                    }
                }

                // Details + Action buttons side by side
                // === DETAILS + ACTIONS ===
                ColumnLayout {
                    Layout.fillWidth: true
                    Layout.leftMargin: 16
                    Layout.rightMargin: 16
                    Layout.bottomMargin: 4
                    spacing: 2
                    visible: proxy && proxy.activeTopic !== ""

                    // Detalles
                    Repeater {
                        model: ListModel {
                            ListElement { label: "Creado"; field: "dtec" }
                            ListElement { label: "Autor"; field: "autr" }
                            ListElement { label: "Idiomas"; field: "idioma" }
                            ListElement { label: "Categoría"; field: "ctgy" }
                            ListElement { label: "Estado"; field: "levelLabel" }
                        }

                        delegate: Text {
                            Layout.fillWidth: true
                            height: visible ? 22 : 0

                            visible: {
                                if (!proxy)
                                    return false

                                if (model.field === "idioma")
                                    return proxy.slng !== ""

                                return proxy[model.field] !== undefined &&
                                       proxy[model.field] !== ""
                            }

                            text: {
                                if (!proxy)
                                    return ""

                                if (model.field === "idioma")
                                    return model.label + ": " +
                                           proxy.slng + " \u2192 " + proxy.tlng

                                return model.label + ": " + proxy[model.field]
                            }

                            font.pixelSize: 10
                            color: "#394651"
                            elide: Text.ElideRight
                        }
                    }

                    // === UNA SOLA LÍNEA DE ACCIONES ===
                    RowLayout {
                        Layout.fillWidth: true
                        Layout.topMargin: 6
                        spacing: 6

                        // Revisión — botón principal
                        Rectangle {
                            Layout.fillWidth: true
                            Layout.maximumWidth: 180
                            height: 30
                            radius: 8

                            color: reviewActionMouse.pressed
                                   ? "#e7ecef"
                                   : "#f1f4f6"

                            border.color: proxy && proxy.actionLabel !== ""
                                          ? "#bdc8cf"
                                          : "#e1e6eb"

                            visible: proxy && proxy.actionLabel !== ""

                            Text {
                                anchors.centerIn: parent
                                text: proxy ? proxy.actionLabel : ""
                                font.pixelSize: 12
                                font.bold: true
                                color: "#3f4d59"
                                elide: Text.ElideRight
                            }

                            MouseArea {
                                id: reviewActionMouse
                                anchors.fill: parent
                                cursorShape: Qt.PointingHandCursor

                                onClicked: {
                                    if (proxy)
                                        proxy.performAction()
                                }
                            }
                        }

                        Item {
                            Layout.fillWidth: true

                        }

                        // Editar
                        Item {
                            width: 38
                            height: 36

                            Rectangle {
                                anchors.fill: parent
                                radius: 8
                                color: editActionMouse.pressed
                                       ? "#e8ecef"
                                       : "transparent"

                                Text {
                                    anchors.centerIn: parent
                                    text: "\u270E"
                                    font.pixelSize: 18
                                    color: "#555"
                                }
                            }

                            MouseArea {
                                id: editActionMouse
                                anchors.fill: parent
                                cursorShape: Qt.PointingHandCursor

                                onClicked: {
                                    // Acción de Editar
                                }
                            }
                        }

                        // Notas
                        Item {
                            width: 38
                            height: 36

                            Rectangle {
                                anchors.fill: parent
                                radius: 8
                                color: notesActionMouse.pressed
                                       ? "#e8ecef"
                                       : "transparent"

                                Text {
                                    anchors.centerIn: parent
                                    text: "\u2637"
                                    font.pixelSize: 19
                                    color: "#555"
                                }
                            }

                            MouseArea {
                                id: notesActionMouse
                                anchors.fill: parent
                                cursorShape: Qt.PointingHandCursor

                                onClicked: {
                                    // Acción de Notas
                                }
                            }
                        }

                        // Compartir
                        Item {
                            width: 38
                            height: 36

                            Rectangle {
                                anchors.fill: parent
                                radius: 8
                                color: shareActionMouse.pressed
                                       ? "#e8ecef"
                                       : "transparent"

                                Text {
                                    anchors.centerIn: parent
                                    text: "\u2197"
                                    font.pixelSize: 20
                                    color: "#555"
                                }
                            }

                            MouseArea {
                                id: shareActionMouse
                                anchors.fill: parent
                                cursorShape: Qt.PointingHandCursor

                                onClicked: {
                                    // Acción de Share
                                }
                            }
                        }

                        // Eliminar
                        Item {
                            width: 38
                            height: 36

                            Rectangle {
                                anchors.fill: parent
                                radius: 8
                                color: deleteActionMouse.pressed
                                       ? "#e8ecef"
                                       : "transparent"

                                Text {
                                    anchors.centerIn: parent
                                    text: "\u2715"
                                    font.pixelSize: 18
                                    color: "#555"
                                }
                            }

                            MouseArea {
                                id: deleteActionMouse
                                anchors.fill: parent
                                cursorShape: Qt.PointingHandCursor

                                onClicked: {
                                    // Acción de Delete
                                }
                            }
                        }
                    }
                }
            }
        }

        Rectangle { Layout.fillWidth: true; height: 1; color: "#dfe5ea" }

        // === TOPIC LIST ===
        ListView {
            id: topicList
            Layout.fillWidth: true; Layout.fillHeight: true
            clip: true
            model: proxy ? proxy.topicModel : null

            Text {
                anchors.centerIn: parent
                text: "No hay topics en este idioma"
                font.pixelSize: 14; color: "#7c8790"
                visible: proxy && proxy.topicCount === 0
            }

            delegate: Item {
                width: topicList.width
                height: 56
                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 16; anchors.rightMargin: 16
                    spacing: 10
                    Image {

                        source: proxy ? proxy.stateImage(model.stts) : ""

                    }
                    Text {
                        Layout.fillWidth: true
                        text: model.name
                        font.pixelSize: 14; color: "#34424d"
                        elide: Text.ElideRight
                    }
                }
                Rectangle {
                    anchors.left: parent.left; anchors.right: parent.right; anchors.bottom: parent.bottom
                    height: 0; color: "#edf0f2"
                }
                MouseArea {
                    anchors.fill: parent
                    onClicked: { if (proxy) proxy.selectTopic(model.name) }
                }
            }
        }
    }
}
