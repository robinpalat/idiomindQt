// PracticeDialog: selector y sesión de las cinco prácticas reales.
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtMultimedia
import Idiomind 1.0

Rectangle {
    id: dialog

    color: "#00000000"

    signal close()

    property var proxy: App.topicProxy
    property var practice: proxy ? proxy.practiceController : null

    property int selectedMode: 0
    property bool setupVisible: false

    function modeName(mode) {
        switch (mode) {
        case 1: return "Tarjetas de vocabulario"
        case 2: return "Multiple choice"
        case 3: return "Reconocer pronunciación"
        case 4: return "Imágenes"
        case 5: return "Escuchar y escribir frases"
        default: return "Práctica"
        }
    }

    function imageUrl(path) {
        if (!path || path === "")
            return ""

        return path.indexOf("://") >= 0
                ? path
                : "file://" + path
    }

    MediaPlayer {
        id: audioPlayer

        audioOutput: AudioOutput {}

        source: practice ? imageUrl(practice.audioSource) : ""

        onSourceChanged: {
            if (source !== "")
                play()
        }
    }

    // ============================================================
    // ESTADOS
    // ============================================================
    //
    // 0 = selector
    // 1 = configuración
    // 2 = práctica activa
    // 3 = resultado
    //
    property int pageIndex:
        practice && practice.finished
            ? 3
            : practice && practice.active
                ? 2
                : setupVisible
                    ? 1
                    : 0


    // ============================================================
    // SELECTOR / CONFIGURACIÓN
    // PANEL INFERIOR — NO TAPA EL FOOTER DE MAIN
    // ============================================================

    Rectangle {
        id: setupPanel

        visible: pageIndex === 0 || pageIndex === 1

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom

        // Deja libre la zona del footer de Main.qml
        anchors.bottomMargin: 48

        // Más alto que PlayDialog por la cantidad de contenido
       height: 400
        color: "#ffffff"

        radius: 10

        border.width: 1
        border.color: "#e2e5e8"
        clip: true
        layer.enabled: true


        z: 10


        // ========================================================
        // X — CERRAR SELECTOR / CONFIGURACIÓN
        // ========================================================

        Row {
            id: setupHeaderActions

            anchors.top: parent.top
            anchors.right: parent.right
            anchors.topMargin: 0
            anchors.rightMargin: 6

            spacing: 2

            // REINICIAR
            Item {
                width: 44
                height: 44

                visible: practice !== null

                Text {
                    anchors.centerIn: parent
                    text: "↻"
                    font.pixelSize: 23
                    color: "#555555"
                }

                MouseArea {
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor

                    onClicked: {
                        if (practice)
                            practice.restartAll()
                    }
                }
            }

            // CERRAR
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
                    cursorShape: Qt.PointingHandCursor

                    onClicked: {
                        setupVisible = false
                        dialog.close()
                    }
                }
            }
        }

        // ========================================================
        // SELECTOR DE PRÁCTICAS
        // ========================================================

        Item {
            anchors.fill: parent

            visible: pageIndex === 0

            ListView {
                anchors.fill: parent

                anchors.topMargin: 44

                clip: true

                model: ListModel {

                    ListElement {
                        mode: 1
                        name: "Tarjetas de vocabulario"
                        desc: "Pregunta, respuesta y autoevaluación"
                        imgFile: "qrc:/images/practice/10.png"
                    }

                    ListElement {
                        mode: 2
                        name: "Multiple choice"
                        desc: "Elige la traducción correcta"
                        imgFile: "qrc:/images/practice/5.png"
                    }

                    ListElement {
                        mode: 3
                        name: "Reconocer pronunciación"
                        desc: "Reconoce y autoevalúa la palabra"
                        imgFile: "qrc:/images/practice/15.png"
                    }

                    ListElement {
                        mode: 4
                        name: "Imágenes"
                        desc: "Asocia la imagen con la palabra"
                        imgFile: "qrc:/images/practice/21.png"
                    }

                    ListElement {
                        mode: 5
                        name: "Escuchar y escribir frases"
                        desc: "Escribe la frase escuchada"
                        imgFile: "qrc:/images/practice/0.png"
                    }
                }


                delegate: Rectangle {
                    width: ListView.view.width
                    height: 68

                    color: modeMouse.pressed
                           ? "#f0f0f0"
                           : "#ffffff"

                    border.width: 0

                    RowLayout {
                        anchors.fill: parent
                        anchors.leftMargin: 16
                        anchors.rightMargin: 16

                        spacing: 8

                        Image {
                            width: 48
                            height: 48

                            source: {
                                var revision = practice
                                        ? practice.practiceRevision
                                        : 0

                                return "qrc:/images/practice/"
                                       + (practice
                                          ? practice.practiceIcon(model.mode)
                                          : 1)
                                       + ".png"
                            }

                            fillMode: Image.PreserveAspectFit

                        }

                        ColumnLayout {
                            Layout.fillWidth: true
                            Layout.alignment: Qt.AlignVCenter
                            spacing: 4

                            Text {
                                Layout.fillWidth: true

                                text: model.name

                                font.pixelSize: 16
                                font.bold: true
                                color: "#333333"

                                horizontalAlignment: Text.AlignLeft
                            }

                            Text {
                                Layout.fillWidth: true

                                text: {
                                    var revision = practice
                                            ? practice.practiceRevision
                                            : 0

                                    return model.desc
                                           + (practice &&
                                              practice.practiceRemaining(model.mode) > 0
                                              ? "  (" +
                                                practice.practiceRemaining(model.mode) +
                                                ")"
                                              : "")
                                }

                                font.pixelSize: 13
                                color: "#888888"

                                horizontalAlignment: Text.AlignLeft
                                wrapMode: Text.Wrap
                            }
                        }
                    }

                    MouseArea {
                        id: modeMouse
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor

                        onClicked: {
                            if (!practice)
                                return

                            selectedMode = model.mode

                            if (practice.hasConfiguration(model.mode)) {
                                practice.start(model.mode)
                            } else {
                                setupVisible = true
                            }
                        }
                    }
                }
            }
        }


        // ========================================================
        // CONFIGURACIÓN
        // ========================================================

        Item {
            anchors.fill: parent

            visible: pageIndex === 1


            ColumnLayout {
                anchors.fill: parent

                anchors.margins: 24

                anchors.topMargin: 52

                spacing: 16


                Text {
                    Layout.fillWidth: true

                    text: "Configuración: "
                          + modeName(selectedMode)

                    font.pixelSize: 19

                    font.bold: true

                    color: "#333333"

                    horizontalAlignment:
                        Text.AlignHCenter
                }


                CheckBox {
                    id: learningCheck

                    Layout.alignment:
                        Qt.AlignHCenter

                    text: "Learning mode: grupos de 10 notas"
                }


                Text {
                    Layout.fillWidth: true

                    text: "Idioma de la pregunta y respuesta"

                    font.pixelSize: 14

                    color: "#555555"

                    horizontalAlignment:
                        Text.AlignHCenter
                }


                ButtonGroup {
                    id: languageGroup
                }


                RadioButton {
                    id: sourceToTargetRadio

                    Layout.alignment:
                        Qt.AlignHCenter

                    text: (proxy && proxy.slng
                           ? proxy.slng
                           : "Source")
                          + " -> "
                          + (proxy && proxy.tlng
                             ? proxy.tlng
                             : "Target")

                    checked: true

                    ButtonGroup.group:
                        languageGroup
                }


                RadioButton {
                    Layout.alignment:
                        Qt.AlignHCenter

                    text: (proxy && proxy.tlng
                           ? proxy.tlng
                           : "Target")
                          + " -> "
                          + (proxy && proxy.slng
                             ? proxy.slng
                             : "Source")

                    ButtonGroup.group:
                        languageGroup
                }


                Item {
                    Layout.fillHeight: true
                }


                RowLayout {
                    Layout.alignment:
                        Qt.AlignHCenter

                    spacing: 12


                    Button {
                        text: "Iniciar"

                        onClicked: {
                            if (practice) {

                                practice.configure(
                                    selectedMode,
                                    learningCheck.checked,
                                    sourceToTargetRadio.checked
                                )

                                setupVisible = false

                                practice.start(selectedMode)
                            }
                        }
                    }


                    Button {
                        text: "Cancelar"

                        onClicked: {
                            setupVisible = false
                        }
                    }
                }
            }
        }
    }


    // ============================================================
    // PRÁCTICA ACTIVA — FULLSCREEN
    // ============================================================

    Rectangle {
        id: activePracticePanel

        visible: pageIndex === 2

        anchors.fill: parent

        color: "#ffffff"

        radius: 0

        z: 20


        // ========================================================
        // X — CERRAR PRÁCTICA FULLSCREEN
        // ========================================================

        Item {
            id: practiceCloseButton

            width: 44
            height: 44

            anchors.top: parent.top
            anchors.right: parent.right

            anchors.topMargin: 8
            anchors.rightMargin: 8

            z: 100


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

                    if (practice)
                        practice.stop()

                    dialog.close()
                }
            }
        }


        // ========================================================
        // CONTENIDO DE LA PRÁCTICA
        // ========================================================

        Flickable {
            anchors.fill: parent

            anchors.topMargin: 8

            clip: true

            contentWidth: width

            contentHeight:
                activeColumn.implicitHeight + 48


            ColumnLayout {
                id: activeColumn

                width:
                    Math.min(parent.width - 40, 620)

                x:
                    (parent.width - width) / 2

                y: 24

                spacing: 16


                Text {
                    Layout.fillWidth: true

                    text:
                        practice &&
                        practice.awaitingGroupDecision
                        ? "Grupo de 10 notas completado"
                        : (practice
                           ? modeName(practice.mode)
                           : "Práctica")

                    font.pixelSize: 18

                    font.bold: true

                    color: "#333333"

                    horizontalAlignment:
                        Text.AlignHCenter
                }


                Text {
                    Layout.fillWidth: true

                    text:
                        practice &&
                        !practice.awaitingGroupDecision
                        ? ("Ronda "
                           + practice.round
                           + "  ·  "
                           + (practice.currentIndex + 1)
                           + "/"
                           + practice.total)
                        : ""

                    font.pixelSize: 12

                    color: "#999999"

                    horizontalAlignment:
                        Text.AlignHCenter
                }


                ProgressBar {
                    Layout.fillWidth: true

                    from: 0
                    to: 100

                    value:
                        practice
                        ? practice.progress
                        : 0
                }


                // =================================================
                // DECISIÓN DE GRUPO
                // =================================================

                ColumnLayout {
                    Layout.fillWidth: true

                    visible:
                        practice &&
                        practice.awaitingGroupDecision

                    spacing: 12


                    Text {
                        Layout.fillWidth: true

                        text:
                            "¿Quieres repetir este grupo o continuar?"

                        font.pixelSize: 16

                        color: "#555555"

                        horizontalAlignment:
                            Text.AlignHCenter
                    }


                    RowLayout {
                        Layout.alignment:
                            Qt.AlignHCenter

                        spacing: 12


                        Button {
                            text: "Repetir"

                            onClicked:
                                practice.again()
                        }


                        Button {
                            text: "Continuar"

                            onClicked:
                                practice.continueGroup()
                        }
                    }
                }


                // =================================================
                // MODOS 1 Y 2
                // =================================================

                Text {
                    Layout.fillWidth: true

                    visible:
                        practice &&
                        !practice.awaitingGroupDecision &&
                        (practice.mode === 1 ||
                         practice.mode === 2)

                    text:
                        practice
                        ? practice.question
                        : ""

                    font.pixelSize:
                        practice &&
                        practice.mode === 1
                        ? 30
                        : 24

                    font.bold: true

                    color: "#222222"

                    horizontalAlignment:
                        Text.AlignHCenter

                    wrapMode:
                        Text.Wrap
                }


                // =================================================
                // PRONUNCIACIÓN
                // =================================================

                Text {
                    Layout.fillWidth: true

                    visible:
                        practice &&
                        !practice.awaitingGroupDecision &&
                        practice.mode === 3

                    text:
                        "¿Reconoces esta palabra?"

                    font.pixelSize: 20

                    color: "#333333"

                    horizontalAlignment:
                        Text.AlignHCenter
                }


                Text {
                    Layout.fillWidth: true

                    visible:
                        practice &&
                        !practice.awaitingGroupDecision &&
                        practice.mode === 3

                    text:
                        practice
                        ? practice.question
                        : ""

                    font.pixelSize: 22

                    color: "#555555"

                    horizontalAlignment:
                        Text.AlignHCenter

                    wrapMode:
                        Text.Wrap
                }


                // =================================================
                // IMÁGENES
                // =================================================

                Image {
                    Layout.alignment:
                        Qt.AlignHCenter

                    visible:
                        practice &&
                        !practice.awaitingGroupDecision &&
                        practice.mode === 4

                    width:
                        Math.min(parent.width, 260)

                    height: 190

                    source:
                        practice
                        ? imageUrl(practice.image)
                        : ""

                    fillMode:
                        Image.PreserveAspectFit
                }


                Text {
                    Layout.fillWidth: true

                    visible:
                        practice &&
                        !practice.awaitingGroupDecision &&
                        practice.mode === 4

                    text:
                        practice
                        ? practice.question
                        : ""

                    font.pixelSize: 24

                    font.bold: true

                    color: "#222222"

                    horizontalAlignment:
                        Text.AlignHCenter

                    wrapMode:
                        Text.Wrap
                }


                // =================================================
                // ESCUCHAR Y ESCRIBIR
                // =================================================

                Text {
                    Layout.fillWidth: true

                    visible:
                        practice &&
                        !practice.awaitingGroupDecision &&
                        practice.mode === 5

                    text:
                        "Escucha y escribe la frase"

                    font.pixelSize: 20

                    color: "#333333"

                    horizontalAlignment:
                        Text.AlignHCenter
                }


                Text {
                    Layout.fillWidth: true

                    visible:
                        practice &&
                        !practice.awaitingGroupDecision &&
                        practice.mode === 5

                    text:
                        practice
                        ? practice.question
                        : ""

                    font.pixelSize: 18

                    color: "#555555"

                    horizontalAlignment:
                        Text.AlignHCenter

                    wrapMode:
                        Text.Wrap
                }


                Button {
                    Layout.alignment:
                        Qt.AlignHCenter

                    visible:
                        practice &&
                        !practice.awaitingGroupDecision &&
                        (practice.mode === 3 ||
                         practice.mode === 5) &&
                        practice.audioSource !== ""

                    text:
                        "Reproducir audio"

                    onClicked:
                        audioPlayer.play()
                }


                TextField {
                    id: sentenceAnswer

                    Layout.fillWidth: true

                    visible:
                        practice &&
                        !practice.awaitingGroupDecision &&
                        practice.mode === 5 &&
                        !practice.answered

                    placeholderText:
                        "Escribe aquí la frase"

                    onAccepted:
                        if (practice)
                            practice.submit(text)
                }


                // =================================================
                // MULTIPLE CHOICE
                // =================================================

                Flow {
                    Layout.fillWidth: true

                    visible:
                        practice &&
                        !practice.awaitingGroupDecision &&
                        practice.mode === 2 &&
                        !practice.answered

                    spacing: 10


                    Repeater {
                        model:
                            practice
                            ? practice.options
                            : []


                        delegate: Button {
                            width:
                                Math.max(
                                    140,
                                    (activeColumn.width - 10) / 2
                                )

                            text: modelData

                            onClicked:
                                if (practice)
                                    practice.submit(index)
                        }
                    }
                }


                // =================================================
                // RESULTADO DE RESPUESTA
                // =================================================

                Text {
                    Layout.fillWidth: true

                    visible:
                        practice &&
                        !practice.awaitingGroupDecision &&
                        practice.answered

                    text:
                        practice
                        ? (practice.result.correct
                           ? "Correcto"
                           : "Respuesta incorrecta")
                        : ""

                    font.pixelSize: 18

                    font.bold: true

                    color:
                        practice &&
                        practice.result.correct
                        ? "#2e8b57"
                        : "#ae3259"

                    horizontalAlignment:
                        Text.AlignHCenter
                }


                Text {
                    Layout.fillWidth: true

                    visible:
                        practice &&
                        !practice.awaitingGroupDecision &&
                        practice.answered &&
                        practice.mode !== 5

                    text:
                        practice
                        ? practice.response
                        : ""

                    font.pixelSize: 18

                    color: "#555555"

                    horizontalAlignment:
                        Text.AlignHCenter

                    wrapMode:
                        Text.Wrap
                }


                Text {
                    Layout.fillWidth: true

                    visible:
                        practice &&
                        !practice.awaitingGroupDecision &&
                        practice.answered &&
                        practice.mode === 5

                    text:
                        practice
                        ? ("Coincidencia: "
                           + practice.result.percentage
                           + "%")
                        : ""

                    font.pixelSize: 16

                    color: "#555555"

                    horizontalAlignment:
                        Text.AlignHCenter
                }


                // =================================================
                // AUTOEVALUACIÓN
                // =================================================

                RowLayout {
                    Layout.alignment:
                        Qt.AlignHCenter

                    visible:
                        practice &&
                        !practice.awaitingGroupDecision &&
                        !practice.answered &&
                        practice.mode !== 2 &&
                        practice.mode !== 5

                    spacing: 12


                    Button {
                        text: "No lo sabía"

                        onClicked:
                            if (practice)
                                practice.mark(false)
                    }


                    Button {
                        text: "Lo sabía"

                        onClicked:
                            if (practice)
                                practice.mark(true)
                    }
                }


                // =================================================
                // EVALUAR
                // =================================================

                RowLayout {
                    Layout.alignment:
                        Qt.AlignHCenter

                    visible:
                        practice &&
                        !practice.awaitingGroupDecision &&
                        practice.mode === 5 &&
                        !practice.answered


                    Button {
                        text: "Evaluar"

                        onClicked:
                            if (practice)
                                practice.submit(
                                    sentenceAnswer.text
                                )
                    }
                }


                // =================================================
                // SIGUIENTE
                // =================================================

                Button {
                    Layout.alignment:
                        Qt.AlignHCenter

                    visible:
                        practice &&
                        !practice.awaitingGroupDecision &&
                        practice.answered

                    text:
                        "Siguiente"

                    onClicked: {
                        sentenceAnswer.clear()

                        if (practice)
                            practice.next()
                    }
                }
            }
        }
    }


    // ============================================================
    // RESULTADO — FULLSCREEN
    // ============================================================

    Rectangle {
        id: resultPanel

        visible: pageIndex === 3

        anchors.fill: parent

        color: "#ffffff"

        z: 20


        // X DEL RESULTADO

        Item {
            width: 44
            height: 44

            anchors.top: parent.top
            anchors.right: parent.right

            anchors.topMargin: 8
            anchors.rightMargin: 8

            z: 100


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
                    if (practice)
                        practice.acknowledgeResult()

                    dialog.close()
                }
            }
        }


        ColumnLayout {
            anchors.fill: parent

            anchors.margins: 24

            spacing: 16


            Item {
                Layout.fillHeight: true
            }


            Image {
                Layout.alignment:
                    Qt.AlignHCenter

                width: 128
                height: 128

                source:
                    "qrc:/images/practice/21.png"

                fillMode:
                    Image.PreserveAspectFit
            }


            Text {
                Layout.fillWidth: true

                text:
                    practice
                    ? modeName(practice.mode)
                    : "Práctica"

                font.pixelSize: 20

                font.bold: true

                color: "#333333"

                horizontalAlignment:
                    Text.AlignHCenter
            }


            Text {
                Layout.fillWidth: true

                text:
                    practice
                    ? ((practice.result.completed
                        ? "Práctica completada"
                        : "Práctica finalizada")
                       + "\n"
                       + practice.correct
                       + "/"
                       + practice.total
                       + "  ·  "
                       + practice.score
                       + "%")
                    : ""

                font.pixelSize: 16

                color: "#666666"

                horizontalAlignment:
                    Text.AlignHCenter
            }


            Item {
                Layout.fillHeight: true
            }
        }
    }
}
