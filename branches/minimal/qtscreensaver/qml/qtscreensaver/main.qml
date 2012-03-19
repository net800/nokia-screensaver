import QtQuick 1.0
import QtMobility.sensors 1.2
//import QtMobility.messaging 1.1

Rectangle {
    id:screen
    width: 360
    height: 640
    color: "#000000"

/*
    MessageModel {
        id: mm
        filter: MessageFilter {
            comparator: MessageFilter.Equal
            type: MessageFilter.Type
            value: MessageFilter.Sms
        }

        onMessageAdded: {
            newMessagesText.text = count + ' unread';
        }
    }
*/

    Text {
        id: newMessagesText
        color: "#FFFFFF"
        font.pointSize: 8
        x: 10
        y: 10
    }


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
        x: 0
        y: 0
        visible: true
        anchors.rightMargin: 0
        anchors.bottomMargin: 0
        anchors.leftMargin: 0
        anchors.topMargin: 0
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
        style: Text.Outline
        styleColor: "#FFFFFF"
    }

    Text {
        id: dateText
        color: "#FFFFFF"
        font.pointSize: 8
        //font.bold: true
    }


    Timer {
        id:clocktimer
        triggeredOnStart: true
        running: true
        repeat: true
        onTriggered: {
            var date = new Date();

            var th = date.getHours();
            var tm = date.getMinutes();

            timeText.y = ((th*60 + tm)/1439) * (screen.height / 2 - 40) + 20;
            dateText.y = timeText.y + timeText.height + 40;

            function format(x) {
                if (x < 10)
                    return '0' + x;
                return x;
            }

            timeText.text = format(th) + ':' + format(tm);
            var day = ['Sun','Mon','Tue','Wed','Thu','Fri','Sat'][date.getDay()];
            var td = format(date.getDate());
            var tM = format(date.getMonth() + 1);
            var ty = format(date.getFullYear());
            dateText.text = day + ' ' + td + '/' + tM + '/' + ty;

            timeText.x = (screen.width - timeText.width)/2;
            dateText.x = (screen.width - dateText.width)/2;

            clocktimer.interval = 1000*(60 - date.getSeconds()) - date.getMilliseconds();
            sensor.active = true;

            //newMessagesText.text = mm.count;
        }
    }
}
