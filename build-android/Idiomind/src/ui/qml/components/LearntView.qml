// F6-B.7: Vista "Aprendidos" — datos reales del topic activo.
import QtQuick
import QtQuick.Layouts
import Idiomind 1.0

Rectangle {
    color: "#fafafa"

    property var proxy: App.topicProxy

    ListView {
        anchors.fill: parent; clip: true
        model: proxy ? proxy.learntModel : null

        // Empty state
        Text {
            anchors.centerIn: parent
            text: proxy && proxy.activeTopic
                ? "No hay ítems aprendidos"
                : "Selecciona un topic"
            font.pixelSize: 14; color: "#999"
            visible: proxy && proxy.learntModel && proxy.learntModel.itemCount === 0
        }

        delegate: Rectangle {
            width: parent ? parent.width : 0; height: 44
            color: index % 2 === 0 ? "#ffffff" : "#fafafa"
            RowLayout {
                anchors.fill: parent; anchors.leftMargin: 16; anchors.rightMargin: 16
                Column {
                    Layout.fillWidth: true
                    Text { text: model.plainText; font.pixelSize: 14; color: "#333" }
                    Text { text: model.srce; font.pixelSize: 11; color: "#999" }
                }
            }
        }
    }
}
