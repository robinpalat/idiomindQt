// F6-A: Diálogo de Reproducción — placeholder con controles visuales.
import QtQuick
import QtQuick.Layouts

Rectangle {
    id: dialog
    color: "#00000080"
    signal close()

    Rectangle {
        anchors.fill: parent; anchors.margins: 32
        color: "#ffffff"; radius: 12

        ColumnLayout {
            anchors.fill: parent; anchors.margins: 20; spacing: 12

            Text { text: "Reproducir"; font.pixelSize: 18; font.bold: true; color: "#333"; Layout.fillWidth: true }

            Rectangle { Layout.fillWidth: true; height: 1; color: "#e0e0e0" }

            // Current item
            ColumnLayout {
                Layout.fillWidth: true; spacing: 4
                Text { text: "Reproduciendo:"; font.pixelSize: 12; color: "#888" }
                Text { text: "hello — hola"; font.pixelSize: 16; font.bold: true; color: "#333" }
            }

            // Controls placeholder
            RowLayout {
                Layout.fillWidth: true; Layout.alignment: Qt.AlignHCenter; spacing: 24
                Rectangle { width: 48; height: 48; radius: 24; color: "#f5f5f5"
                    Text { anchors.centerIn: parent; text: "|◀"; font.pixelSize: 16; color: "#666" }
                }
                Rectangle { width: 64; height: 64; radius: 32; color: "#3b82f6"
                    Text { anchors.centerIn: parent; text: "▶"; font.pixelSize: 20; color: "white" }
                }
                Rectangle { width: 48; height: 48; radius: 24; color: "#f5f5f5"
                    Text { anchors.centerIn: parent; text: "▶|"; font.pixelSize: 16; color: "#666" }
                }
            }

            // Options
            RowLayout { Layout.fillWidth: true; spacing: 12
                Rectangle { Layout.fillWidth: true; height: 36; radius: 4; color: "#f5f5f5"
                    Text { anchors.centerIn: parent; text: "Palabras"; font.pixelSize: 12; color: "#555" }
                }
                Rectangle { Layout.fillWidth: true; height: 36; radius: 4; color: "#e8f5e9"
                    Text { anchors.centerIn: parent; text: "Oraciones"; font.pixelSize: 12; color: "#2e7d32" }
                }
            }

            Item { Layout.fillHeight: true }

            Rectangle {
                Layout.fillWidth: true; height: 40; radius: 6; color: "#f5f5f5"
                Text { anchors.centerIn: parent; text: "Cerrar"; font.pixelSize: 13; color: "#666" }
                MouseArea { anchors.fill: parent; onClicked: dialog.close() }
            }
        }
    }
}
