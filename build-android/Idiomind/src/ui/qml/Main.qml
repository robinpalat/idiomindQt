// F6-A.1.1: Main.qml — Toolbar + Strips + Actions + Content.
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import Idiomind 1.0

Window {
    id: root
    width: 420; height: 740
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

    ColumnLayout {
        anchors.fill: parent; spacing: 0

        // === TOOLBAR SUPERIOR ===
        Rectangle {
            Layout.fillWidth: true; height: 44
            color: "#ffffff"
            border.color: "#e0e0e0"; border.width: 1

            RowLayout {
                anchors.fill: parent; anchors.leftMargin: 8; anchors.rightMargin: 12; spacing: 0

                // Hamburger ☰
                Item {
                    width: 36; height: parent.height
                    ColumnLayout {
                        anchors.centerIn: parent; spacing: 3
                        Rectangle { width: 16; height: 2; radius: 1; color: "#555" }
                        Rectangle { width: 16; height: 2; radius: 1; color: "#555" }
                        Rectangle { width: 16; height: 2; radius: 1; color: "#555" }
                    }
                    MouseArea { anchors.fill: parent; onClicked: drawerOpen = true }
                }

                // Center title
                Item {
                    Layout.fillWidth: true; Layout.fillHeight: true
                    Text {
                        anchors.centerIn: parent
                        text: {
                            switch (currentTab) {
                            case 0: return "Aprendiendo"
                            case 1: return "Aprendidos"
                            case 2: return "Notas"
                            case 3: return "Administrar"
                            }
                        }
                        font.pixelSize: 15; font.bold: true; color: "#333"
                    }
                }

                // Tareas button
                Item {
                    width: 60; height: parent.height
                    RowLayout {
                        anchors.centerIn: parent; spacing: 4
                        Text { text: "Tareas"; font.pixelSize: 12; color: "#555" }
                        Text {
                            text: tasksExpanded ? "v" : ">"
                            font.pixelSize: 12; color: "#888"
                            rotation: tasksExpanded ? 90 : 0
                            Behavior on rotation { NumberAnimation { duration: 150 } }
                        }
                    }
                    MouseArea { anchors.fill: parent; onClicked: tasksExpanded = !tasksExpanded }
                }
            }
        }

        // === TAREAS PANEL (expandible hacia abajo) ===
        Rectangle {
            Layout.fillWidth: true
            height: tasksExpanded ? root.height * 0.5 : 0
            color: "#ffffff"
            border.color: "#e0e0e0"; border.width: 1
            clip: true

            Behavior on height { NumberAnimation { duration: 200; easing.type: Easing.InOutQuad } }

            ListView {
                anchors.fill: parent; anchors.margins: 8
                model: proxy ? proxy.taskModel : null
                Text {
                    anchors.centerIn: parent
                    text: "No hay tareas pendientes"
                    font.pixelSize: 13; color: "#999"
                    visible: proxy && proxy.taskModel && proxy.taskModel.taskCount === 0
                }
                delegate: Rectangle {
                    width: parent.width; height: 36
                    color: index % 2 === 0 ? "#fafafa" : "#ffffff"
                    RowLayout {
                        anchors.fill: parent; anchors.leftMargin: 16; anchors.rightMargin: 16
                        Rectangle {
                            width: 24; height: 18; radius: 4; color: model.color
                            Text { anchors.centerIn: parent; text: model.tag; font.pixelSize: 9; color: "white"; font.bold: true }
                        }
                        Text { text: model.title; font.pixelSize: 13; color: "#333"; Layout.fillWidth: true }
                    }
                }
            }
        }

        // === NAVIGATION STRIPS ===
        NavigationStrips {
            Layout.fillWidth: true
            currentTab: root.currentTab
            onTabChanged: { root.currentTab = tab; swipeView.currentIndex = tab }
        }

        // === CONTENT ===
        SwipeView {
            id: swipeView
            Layout.fillWidth: true; Layout.fillHeight: true
            currentIndex: root.currentTab
            onCurrentIndexChanged: root.currentTab = currentIndex

            LearningView { id: learningView; onOpenViewer: function(index) { viewerIndex = index; viewerOpen = true } }
            LearntView { id: learntView }
            NotesView { id: notesView }
            ManageView { id: manageView; onOpenTopics: topicsModalOpen = true }
        }

        // === CONTEXTUAL FOOTER ===
        ContextualFooter {
            Layout.fillWidth: true
            currentTab: root.currentTab
            onPlay: playDialogOpen = true
            onPractice: practiceDialogOpen = true
            onOpenTopics: topicsModalOpen = true
        }
    }

    // --- Drawer ---
    AppDrawer {
        id: appDrawer; parent: root.contentItem
        width: root.width * 0.75; height: root.height
        open: drawerOpen; onClose: drawerOpen = false
        onImportTopic: importDialog.open()
    }
    Rectangle { anchors.fill: parent; color: "#00000040"; z: 99; visible: drawerOpen; MouseArea { anchors.fill: parent; onClicked: drawerOpen = false } }

    // --- Import .idmnd dialog ---
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

    // --- Modals ---
    TopicsModal { anchors.fill: parent; visible: topicsModalOpen; onClose: topicsModalOpen = false; onImportTopic: importDialog.open() }
    PlayDialog { anchors.fill: parent; visible: playDialogOpen; onClose: playDialogOpen = false }
    PracticeDialog { anchors.fill: parent; visible: practiceDialogOpen; onClose: practiceDialogOpen = false }
    ViewerDialog { anchors.fill: parent; visible: viewerOpen; itemIndex: viewerIndex; onClose: viewerOpen = false }
}
