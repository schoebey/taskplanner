import QtQuick 2.9
import QtQuick.Controls 2.2

ApplicationWindow {
    visible: true
    width: 640
    height: 480
    title: qsTr("Scroll")

    Rectangle {
        color: "green"
        width: 100
        height: parent.height

        Text {
            anchors.centerIn: parent
            text: "Hello, World!"
        }

        Rectangle { color: "blue"; width: parent.width; height: 50; Text { anchors.centerIn: parent; text: "header"; } }
    }
}
