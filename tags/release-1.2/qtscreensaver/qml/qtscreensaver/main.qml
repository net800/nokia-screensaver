import QtQuick 1.0
import QtMobility.sensors 1.2

Rectangle {
    id:screen
    width: 360
    height: 640
    color: "#000000"

    ProximitySensor {
        id: sensor
        active: true
        onReadingChanged: {
            var x = reading.close;
            timeText.visible = dateText.visible = !x
            if (!x)
                sensor.active = false;
        }
    }

    MouseArea {
        anchors.fill: parent
        onClicked: {
            Qt.quit();
        }
    }

    Text {
        id: timeText
        color: "#000000"
        font.pointSize: 40
        font.bold: true
        font.family: "Nokia Sans S60"
        style: Text.Outline
        styleColor: "#FFFFFF"
    }

    Text {
        id: dateText
        color: "#FFFFFF"
        font.family: "Nokia Sans S60"
        font.pointSize: 8
    }

    Timer {
        id:clocktimer
        triggeredOnStart: true
        running: true
        repeat: true
        onTriggered: {
            var date = new Date();

            timeText.y = ((date.getHours()*60 + date.getMinutes())/1439) * (screen.height / 2 - 40) + 20;
            dateText.y = timeText.y + timeText.font.pixelSize + 40;

            timeText.text = Qt.formatTime(date, "hh:mm");
            dateText.text = Qt.formatDate(date, "ddd dd/MM/yyyy");

            timeText.x = (screen.width - timeText.width)/2;
            dateText.x = (screen.width - dateText.width)/2;

            clocktimer.interval = 1000*(60 - date.getSeconds()) - date.getMilliseconds();
            sensor.active = true;
        }
    }
}
