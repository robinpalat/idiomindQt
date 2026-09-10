// F6-B.7: Vista "Aprendidos" — elementos learnt y ciclo de revisión.
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Idiomind 1.0

Rectangle {
    color: "#fafafa"

    property var proxy: App.topicProxy
    property int noteCount: learntList.count

    ColumnLayout {
        anchors.fill: parent
        spacing: 0
        ListView {
            id: learntList
            Layout.fillWidth: true; Layout.fillHeight: true
            clip: true
            model: proxy ? proxy.learntModel : null

            Text {
                anchors.centerIn: parent
                text: proxy && proxy.activeTopic
                    ? "No hay ítems aprendidos"
                    : "Selecciona un topic"
                font.pixelSize: 14; color: "#999"
                visible: proxy && proxy.learntModel && proxy.learntModel.itemCount === 0
            }

            delegate: Rectangle {
                width: learntList.width
                height: 44
                color: index % 2 === 0 ? "#ffffff" : "#fafafa"
                Text {
                    anchors.left: parent.left; anchors.right: parent.right
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.leftMargin: 16; anchors.rightMargin: 16
                    text: model.plainText
                    font.pixelSize: 14; color: "#333"
                    elide: Text.ElideRight
                }
            }
        }
    }
}
