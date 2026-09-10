// F6-A.1.1: Main.qml — Toolbar + Strips + Actions + Content.
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import Idiomind 1.0

Window {
    id: root
    width: 530; height: 600
    minimumWidth: 320; minimumHeight: 560
    visible: true
    title: App.name()
    color: "#fafafa"

    property int currentTab: 0
    property bool tasksExpanded: false
    property bool topicsModalOpen: false
    property bool playDialogOpen: false
    property bool practiceDialogOpen: false
    property bool drawerOpen: false
    property bool viewerOpen: false
    property int viewerIndex: 0
    property var proxy: App.topicProxy

    Component.onCompleted: {
        if (proxy) proxy.refreshTasks()
    }

    Connections {
        target: proxy
        function onTaskActionRequested(action) {
            tasksPopup.close()
            if (action === "play") {
                playDialogOpen = true
            } else if (action === "practice") {
                practiceDialogOpen = true
            } else if (action === "help") {
                Qt.openUrlExternally("https://idiomind.sourceforge.io/help.html")
            } else {
                root.currentTab = action === "learnt" ? 1 : 0
                swipeView.currentIndex = root.currentTab
            }
        }
    }

    Connections {
        target: proxy ? proxy.practiceController : null
        function onFinished() {
            if (proxy) proxy.refreshTasks()
        }
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        // FRANJA SUPERIOR
        Rectangle {
            Layout.fillWidth: true
            height: 2
            color: proxy.topicLineColor || "#62ACBA"
        }

        // TOOLBAR
        Rectangle {
            id: toolbar
            Layout.fillWidth: true
            height: 48
            color: "#ffffff"
            border.color: "#e0e0e0"
            border.width: 1

            ColumnLayout {
                anchors.fill: parent
                spacing: 0

                // PESTAÑAS
                RowLayout {
                    id: toolbarRow
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    spacing: 0

                    RowLayout {
                        id: tabsRow
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        spacing: 2

                        Repeater {
                            model: ListModel {
                                ListElement { tab: 0; label: "Aprendiendo" }
                                ListElement { tab: 1; label: "Aprendidos" }
                                ListElement { tab: 2; label: "Administrar" }
                            }

                            delegate: Rectangle {
                                Layout.fillWidth: true
                                Layout.fillHeight: true

                                color: root.currentTab === model.tab
                                       ? "#eef2f5"
                                       : "transparent"

                                Text {
                                    anchors.centerIn: parent

                                    text: {
                                        if (model.tab === 0)
                                            return "Learning (" + learningView.noteCount + ")"

                                        if (model.tab === 1)
                                            return "Learnt (" + learntView.noteCount + ")"

                                        return "Administrar"
                                    }

                                    font.pixelSize: 12
                                    font.bold: root.currentTab === model.tab
                                    color: "#34424d"
                                }

                                MouseArea {
                                    anchors.fill: parent
                                    onClicked: {
                                        root.currentTab = model.tab
                                        swipeView.currentIndex = model.tab
                                    }
                                }
                            }
                        }
                    }

                    // Botón derecho
                    Item {
                        width: 34
                        Layout.fillHeight: true
                        visible: root.currentTab === 2

                        Text {
                            anchors.centerIn: parent
                            text: "\u22EE"
                            font.pixelSize: 20
                            color: "#53616c"
                        }

                        MouseArea {
                            anchors.fill: parent
                            onClicked: drawerOpen = true
                        }
                    }

                    Item {
                        width: 34
                        Layout.fillHeight: true
                        visible: root.currentTab !== 2

                        Text {
                            anchors.centerIn: parent
                            text: "\uD83D\uDD14"
                            font.pixelSize: 18
                            color: "#53616c"
                        }

                        Rectangle {
                            anchors.right: parent.right
                            anchors.top: parent.top
                            anchors.rightMargin: 1
                            anchors.topMargin: 5
                            width: 15
                            height: 15
                            radius: 8
                            color: "#c85d56"

                            visible: proxy && proxy.taskModel &&
                                     proxy.taskModel.taskCount > 0

                            Text {
                                anchors.centerIn: parent
                                text: proxy.taskModel.taskCount < 10
                                      ? proxy.taskModel.taskCount
                                      : "9+"
                                font.pixelSize: 8
                                font.bold: true
                                color: "white"
                            }
                        }

                        MouseArea {
                            anchors.fill: parent
                            onClicked: tasksPopup.open()
                        }
                    }
                }

                // LÍNEA SEMÁNTICA
                Item {
                    id: semanticLineArea
                    Layout.fillWidth: true
                    height: 4
                    clip: true

                    Rectangle {
                        id: semanticLine
                        height: 4
                        color: proxy.topicLineColor || "#62ACBA"

                        x: root.currentTab === 0
                           ? 0
                           : root.currentTab === 1
                               ? tabsRow.width / 3 + 2
                               : (tabsRow.width / 3 + 2) * 2

                        width: root.currentTab === 0
                               ? semanticLineArea.width
                               : (tabsRow.width - 4) / 3

                        Behavior on x {
                            NumberAnimation { duration: 160 }
                        }

                        Behavior on width {
                            NumberAnimation { duration: 160 }
                        }
                    }
                }
            }
        }

        // CONTENIDO
        SwipeView {
            id: swipeView
            Layout.fillWidth: true
            Layout.fillHeight: true

            currentIndex: root.currentTab

            onCurrentIndexChanged: {
                root.currentTab = currentIndex
            }

            LearningView {
                id: learningView
                onOpenViewer: function(index) {
                    viewerIndex = index
                    viewerOpen = true
                }
            }

            LearntView {
                id: learntView
            }

            ManageView {
                id: manageView
                onOpenTopics: topicsModalOpen = true
            }
        }

        // FOOTER
        ContextualFooter {
            Layout.fillWidth: true
            currentTab: root.currentTab
            onPlay: playDialogOpen = true
            onPractice: practiceDialogOpen = true
            onOpenTopics: topicsModalOpen = true
        }
    }

    // --- Popup de tareas ---
    Popup {
        id: tasksPopup
        x: Math.max(8, root.width - width - 8)
        y: toolbarRow.y + toolbarRow.height + 4
        width: Math.min(300, root.width - 16)
        padding: 8
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
        background: Rectangle { radius: 10; color: "#ffffff"; border.color: "#dfe5ea" }
        contentItem: ListView {
            implicitHeight: Math.min(contentHeight, 280)
            clip: true
            model: proxy ? proxy.taskModel : null
            Text {
                anchors.centerIn: parent
                text: "No hay tareas pendientes"
                font.pixelSize: 13; color: "#87939e"
                visible: proxy && proxy.taskModel && proxy.taskModel.taskCount === 0
            }
            delegate: Rectangle {
                width: ListView.view.width; height: 44; radius: 6
                color: taskMouse.pressed ? "#eef2f5" : "transparent"
                RowLayout {
                    anchors.fill: parent; anchors.leftMargin: 8; anchors.rightMargin: 8; spacing: 8
                    Rectangle {
                        width: 30; height: 20; radius: 5; color: model.color
                        Text { anchors.centerIn: parent; text: model.tag; font.pixelSize: 9; font.bold: true; color: "white" }
                    }
                    Text { Layout.fillWidth: true; text: model.title; font.pixelSize: 12; color: "#34424d"; elide: Text.ElideRight }
                }
                MouseArea {
                    id: taskMouse; anchors.fill: parent
                    onClicked: {
                        if (proxy) proxy.activateTask(model.action, model.topic)
                    }
                }
            }
        }
    }

    // --- Drawer ---
    AppDrawer {
        id: appDrawer; parent: root.contentItem
        width: root.width * 0.75; height: root.height
        open: drawerOpen; onClose: drawerOpen = false
        onImportTopic: importDialog.open()
    }
    Rectangle {
        anchors.fill: parent; color: "#00000040"; z: 99
        visible: drawerOpen
        MouseArea { anchors.fill: parent; onClicked: drawerOpen = false }
    }

    // --- Import .idmnd ---
    FileDialog {
        id: importDialog
        title: "Seleccionar archivo .idmnd"
        nameFilters: ["Archivos Idiomind (*.idmnd)", "Todos los archivos (*)"]
        onAccepted: {
            var path = selectedFile.toString()
            if (proxy) {
                var ok = proxy.importIdmnd(path)
                if (ok) console.log("Topic importado correctamente")
                else console.log("Error al importar topic")
            }
        }
    }

    // --- Modales ---
    TopicsModal { anchors.fill: parent; visible: topicsModalOpen; onClose: topicsModalOpen = false; onImportTopic: importDialog.open() }
    PlayDialog { anchors.fill: parent; visible: playDialogOpen; onClose: playDialogOpen = false }
    PracticeDialog { anchors.fill: parent; visible: practiceDialogOpen; onClose: practiceDialogOpen = false }
    ViewerDialog { anchors.fill: parent; visible: viewerOpen; itemIndex: viewerIndex; onClose: viewerOpen = false }
}