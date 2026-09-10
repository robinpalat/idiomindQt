// F6-A.1.1: 4 franjas independientes con gaps y colores mates.
import QtQuick
import QtQuick.Layouts

Item {
    id: strip
    property int currentTab: 0
    signal tabChanged(int tab)
    implicitHeight: 12

    property var colors: ["#c0392b", "#27ae60", "#f39c12", "#2980b9"]
    property int activeHeight: 14
    property int inactiveHeight: 10

    Row {
        anchors.fill: parent
        spacing: 6

        Repeater {
            model: 4

            Rectangle {
                y: (parent.height - height) / 2
                width: (parent.width - 3 * 6) / 4
                height: index === strip.currentTab ? strip.activeHeight : strip.inactiveHeight
                radius: 3
                color: strip.colors[index]

                Behavior on height { NumberAnimation { duration: 120; easing.type: Easing.InOutQuad } }

                MouseArea {
                    anchors.fill: parent
                    onClicked: strip.tabChanged(index)
                }
            }
        }
    }
}
