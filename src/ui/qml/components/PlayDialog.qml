// PlayDialog: vista de una sesión de reproducción controlada por C++.
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Idiomind 1.0

Rectangle {
    id: dialog
    color: "#00000000"
    signal close()

    property var proxy: App.topicProxy
    property var playback: proxy ? proxy.playbackController : null
    property bool setup: true

    onVisibleChanged: {
        if (visible)
            setup = !playback || !playback.active
    }

    // ============================================================
    // CONFIGURACIÓN
    // ============================================================
    Rectangle {
        id: setupPanel

        visible: setup

        width: Math.min(parent.width)
        height: setupContent.implicitHeight + 40

        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 48

        color: "#ffffff"
        radius: 10

        border.width: 1
        border.color: "#e2e5e8"

        // Sombra sutil
        layer.enabled: true

        ColumnLayout {
            id: setupContent

            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.margins: 20

            spacing: 12

            // Encabezado
            RowLayout {
                Layout.fillWidth: true

                Text {
                    text: "Reproducir"
                    font.pixelSize: 15
                    font.bold: true
                    color: "#333333"
                    Layout.fillWidth: true
                }

                Item {
                    width: 32
                    height: 32

                    Text {
                        anchors.centerIn: parent
                        text: "×"
                        font.pixelSize: 24
                        font.weight: Font.Light
                        color: "#666666"
                    }

                    MouseArea {
                        anchors.fill: parent

                        onClicked: {
                            if (playback)
                                playback.stop()

                            dialog.close()
                        }
                    }
                }
            }

            Rectangle {
                Layout.fillWidth: true
                height: 1
                color: "#eeeeee"
            }

            // Contenido
            ColumnLayout {
                Layout.fillWidth: true
                spacing: 5

                Text {
                    text: "Contenido"
                    font.pixelSize: 11
                    font.bold: true
                    color: "#7a858e"
                }

                GridLayout {
                    Layout.fillWidth: true
                    columns: 2
                    rowSpacing: 0
                    columnSpacing: 20

                    CheckBox {
                        id: words
                        text: "Palabras"
                        checked: true
                    }

                    CheckBox {
                        id: sentences
                        text: "Oraciones"
                        checked: true
                    }

                    CheckBox {
                        id: marked
                        text: "Marcadas"
                    }

                    CheckBox {
                        id: learning
                        text: "Aprendiendo"
                        checked: true
                    }

                    CheckBox {
                        id: difficult
                        text: "Difíciles"
                    }
                }
            }

            // Opciones
            RowLayout {
                Layout.fillWidth: true
                spacing: 20

                CheckBox {
                    id: audioEnabled
                    text: "Audio"
                    checked: true
                }

                CheckBox {
                    id: repeatList
                    text: "Repetir"
                }

                Item {
                    Layout.fillWidth: true
                }

                Button {
                    text: "Iniciar"

                    onClicked: {
                        if (playback) {
                            playback.repeat = repeatList.checked
                            playback.audioEnabled = audioEnabled.checked

                            setup = !playback.start({
                                words: words.checked,
                                sentences: sentences.checked,
                                marked: marked.checked,
                                learning: learning.checked,
                                difficult: difficult.checked
                            })
                        }
                    }
                }
            }
        }
    }


    // ============================================================
    // REPRODUCCIÓN FULLSCREEN
    // ============================================================
    Rectangle {
        id: playbackPanel

        visible: !setup

        anchors.fill: parent

        color: "#ffffff"
        radius: 0

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 20
            spacing: 0

            // Cerrar
            RowLayout {
                Layout.fillWidth: true
                height: 44

                Item {
                    Layout.fillWidth: true
                }

                Item {
                    width: 44
                    height: 44

                    Text {
                        anchors.centerIn: parent
                        text: "×"
                        font.pixelSize: 28
                        font.weight: Font.Light
                        color: "#555555"
                    }

                    MouseArea {
                        anchors.fill: parent

                        onClicked: {
                            if (playback)
                                playback.stop()

                            dialog.close()
                        }
                    }
                }
            }

            // Zona central
            Item {
                Layout.fillWidth: true
                Layout.fillHeight: true

                Column {
                    anchors.centerIn: parent
                    anchors.verticalCenterOffset: -50

                    width: parent.width
                    spacing: 12

                    Text {
                        width: parent.width

                        text: playback ? playback.target : ""

                        font.pixelSize: 28
                        font.bold: true
                        color: "#222222"

                        horizontalAlignment: Text.AlignHCenter
                        wrapMode: Text.Wrap
                    }

                    Text {
                        width: parent.width

                        text: playback ? playback.source : ""

                        font.pixelSize: 18
                        color: "#555555"

                        horizontalAlignment: Text.AlignHCenter
                        wrapMode: Text.Wrap
                    }

                    Text {
                        width: parent.width

                        text: playback
                            ? ((playback.currentIndex + 1) +
                               " / " +
                               playback.totalItems)
                            : ""

                        font.pixelSize: 12
                        color: "#87939e"

                        horizontalAlignment: Text.AlignHCenter
                    }
                }
            }

            // Controles
            RowLayout {
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignHCenter

                height: 52
                spacing: 34

                // Anterior
                Item {
                    width: 52
                    height: 52
                    visible: playback && !playback.playing

                    Text {
                        anchors.centerIn: parent
                        text: "‹"
                        font.pixelSize: 34
                        font.weight: Font.Light
                        color: "#222222"
                    }

                    MouseArea {
                        anchors.fill: parent
                        onClicked: if (playback) playback.previous()
                    }
                }

                // Play
                Item {
                    width: 52
                    height: 52
                    visible: playback && !playback.playing

                    Text {
                        anchors.centerIn: parent
                        text: "▶"
                        font.pixelSize: 25
                        color: "#222222"
                    }

                    MouseArea {
                        anchors.fill: parent
                        onClicked: if (playback) playback.play()
                    }
                }

                // Stop
                Item {
                    width: 52
                    height: 52
                    visible: playback && playback.playing

                    Text {
                        anchors.centerIn: parent
                        text: "■"
                        font.pixelSize: 18
                        color: "#222222"
                    }

                    MouseArea {
                        anchors.fill: parent
                        onClicked: if (playback) playback.stop()
                    }
                }

                // Siguiente
                Item {
                    width: 52
                    height: 52
                    visible: playback && !playback.playing

                    Text {
                        anchors.centerIn: parent
                        text: "›"
                        font.pixelSize: 34
                        font.weight: Font.Light
                        color: "#222222"
                    }

                    MouseArea {
                        anchors.fill: parent
                        onClicked: if (playback) playback.next()
                    }
                }
            }
        }
    }
}