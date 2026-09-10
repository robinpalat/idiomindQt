// F6-C.0: Vista "Notas" — editor real del topic activo.
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Idiomind 1.0

Rectangle {
    color: "#fafafa"

    property var proxy: App.topicProxy
    property string currentNote: proxy ? proxy.note : ""

    onCurrentNoteChanged: {
        if (noteArea.text !== currentNote)
            noteArea.text = currentNote
    }

    Connections {
        target: proxy
        function onActiveTopicChanged() {
            if (proxy) currentNote = proxy.note
        }
    }

    Flickable {
        id: flickable
        anchors.fill: parent; clip: true
        contentWidth: width
        contentHeight: noteArea.height
        flickableDirection: Flickable.VerticalFlick

        // Allow TextArea to handle its own scrolling
        boundsBehavior: Flickable.StopAtBounds

        TextArea {
            id: noteArea
            width: flickable.width
            placeholderText: proxy && proxy.activeTopic
                ? "Escribe la nota aquí..."
                : "Selecciona un topic primero"
            font.pixelSize: 14; color: "#333"
            wrapMode: TextArea.Wrap
            text: currentNote
            readOnly: false
            selectByMouse: true
            onActiveFocusChanged: {
                if (!activeFocus && proxy) {
                    proxy.saveNote(text)
                }
            }
            onTextChanged: {
                // Update currentNote only if user is typing (not from topic change)
                if (activeFocus && text !== currentNote) {
                    currentNote = text
                }
            }
        }
    }
}
