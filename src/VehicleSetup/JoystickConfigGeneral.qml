/****************************************************************************
 *
 * (c) 2009-2020 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Layouts

import QGroundControl
import QGroundControl.Palette
import QGroundControl.Controls
import QGroundControl.ScreenTools
import QGroundControl.Controllers
import QGroundControl.FactSystem
import QGroundControl.FactControls

Item {
    width:  mainCol.width  + (ScreenTools.defaultFontPixelWidth  * 2)
    height: mainCol.height + (ScreenTools.defaultFontPixelHeight * 2)

    readonly property real axisMonitorWidth: ScreenTools.defaultFontPixelWidth * 32

    property bool _buttonsOnly:(function(){
        var isButtonOnly = false
        for(var i = 0; i < _activeJoysticksList.lenght; i++){
            if(_activeJoysticksList[i].axisCount === 0){
                isButtonOnly = true
            }
        }
        return isButtonOnly
    })
    property bool _requiresCalibration:(function(){
        var isCalibrated = false
        for(var i = 0; i < _activeJoysticksList.lenght; i++){
            if(!_activeJoysticksList[i].calibrated && !_buttonsOnly){
                isCalibrated = true
            }
        }
        return isCalibrated
    })

    Column {
        id:                 mainCol
        anchors.centerIn:   parent
        spacing:            ScreenTools.defaultFontPixelHeight
        GridLayout {
            columns:            2
            columnSpacing:      ScreenTools.defaultFontPixelWidth
            rowSpacing:         ScreenTools.defaultFontPixelHeight
            //---------------------------------------------------------------------
            //-- Enable Joystick
            QGCLabel {
                text:               _requiresCalibration ? qsTr("Enable not allowed (Calibrate First)") : qsTr("Enable peripherals input")
                Layout.alignment:   Qt.AlignVCenter
                Layout.minimumWidth: ScreenTools.defaultFontPixelWidth * 36
            }
            QGCCheckBox {
                id:             enabledSwitch
                enabled:        !_requiresCalibration
                onClicked:      {
                    globals.activeVehicle.joystickEnabled = checked
                    globals.activeVehicle.saveJoystickSettings()
                }
                Component.onCompleted: {
                    checked = globals.activeVehicle.joystickEnabled
                }
                Connections {
                    target: globals.activeVehicle
                    onJoystickEnabledChanged: {
                        enabledSwitch.checked = globals.activeVehicle.joystickEnabled
                    }
                }
                Connections {
                    target: joystickManager
                    onActiveJoystickChanged: {
                        if(_activeJoystick) {
                            enabledSwitch.checked = Qt.binding(function() { return _activeJoystick.calibrated && globals.activeVehicle.joystickEnabled })
                        }
                    }
                }
            }
            //---------------------------------------------------------------------
            //-- Selected peripheral list
            QGCLabel {
                text:               qsTr("Selected peripheral:")
                Layout.alignment:   Qt.AlignVCenter
            }
            GridLayout {
                columns: 1
                columnSpacing:      ScreenTools.defaultFontPixelWidth
                rowSpacing:         ScreenTools.defaultFontPixelHeight
                Repeater {
                    model: joystickManager.activePeripherals
                    Row {
                        QGCLabel{
                            id: peripheralName
                            text: joystickManager.activePeripherals[index].name
                        }
                    }
                }
            }
            //---------------------------------------------------------------------
            //-- RC Mode
            QGCLabel {
                text:               qsTr("RC Mode:")
                Layout.alignment:   Qt.AlignVCenter
                visible:            !_buttonsOnly
            }
            Row {
                spacing:            ScreenTools.defaultFontPixelWidth
                visible:            !_buttonsOnly
                QGCRadioButton {
                    text:       "1"
                    checked:    controller.transmitterMode === 1
                    enabled:    !controller.calibrating
                    onClicked:  controller.transmitterMode = 1
                    anchors.verticalCenter: parent.verticalCenter
                }
                QGCRadioButton {
                    text:       "2"
                    checked:    controller.transmitterMode === 2
                    enabled:    !controller.calibrating
                    onClicked:  controller.transmitterMode = 2
                    anchors.verticalCenter: parent.verticalCenter
                }
                QGCRadioButton {
                    text:       "3"
                    checked:    controller.transmitterMode === 3
                    enabled:    !controller.calibrating
                    onClicked:  controller.transmitterMode = 3
                    anchors.verticalCenter: parent.verticalCenter
                }
                QGCRadioButton {
                    text:       "4"
                    checked:    controller.transmitterMode === 4
                    enabled:    !controller.calibrating
                    onClicked:  controller.transmitterMode = 4
                    anchors.verticalCenter: parent.verticalCenter
                }
            }
        }
        Row {
            spacing:                ScreenTools.defaultFontPixelWidth
            //---------------------------------------------------------------------
            //-- Axis Monitors
            Rectangle {
                id:                 axisRect
                color:              Qt.rgba(0,0,0,0)
                border.color:       qgcPal.text
                border.width:       1
                radius:             ScreenTools.defaultFontPixelWidth * 0.5
                width:              axisGrid.width  + (ScreenTools.defaultFontPixelWidth  * 2)
                height:             axisGrid.height + (ScreenTools.defaultFontPixelHeight * 2)
                visible:            !_buttonsOnly
                GridLayout {
                    id:                 axisGrid
                    columns:            2
                    columnSpacing:      ScreenTools.defaultFontPixelWidth
                    rowSpacing:         ScreenTools.defaultFontPixelHeight
                    anchors.centerIn:   parent
                    QGCLabel {
                        text:               globals.activeVehicle.sub ? qsTr("Lateral") : qsTr("Roll")
                        Layout.minimumWidth: ScreenTools.defaultFontPixelWidth * 12
                    }
                    AxisMonitor {
                        id:                 rollAxis
                        height:             ScreenTools.defaultFontPixelHeight
                        width:              axisMonitorWidth
                        mapped:             controller.rollAxisMapped
                        reversed:           controller.rollAxisReversed
                    }

                    QGCLabel {
                        id:                 pitchLabel
                        width:              _attitudeLabelWidth
                        text:               globals.activeVehicle.sub ? qsTr("Forward") : qsTr("Pitch")
                    }
                    AxisMonitor {
                        id:                 pitchAxis
                        height:             ScreenTools.defaultFontPixelHeight
                        width:              axisMonitorWidth
                        mapped:             controller.pitchAxisMapped
                        reversed:           controller.pitchAxisReversed
                    }

                    QGCLabel {
                        id:                 yawLabel
                        width:              _attitudeLabelWidth
                        text:               qsTr("Yaw")
                    }
                    AxisMonitor {
                        id:                 yawAxis
                        height:             ScreenTools.defaultFontPixelHeight
                        width:              axisMonitorWidth
                        mapped:             controller.yawAxisMapped
                        reversed:           controller.yawAxisReversed
                    }

                    QGCLabel {
                        id:                 throttleLabel
                        width:              _attitudeLabelWidth
                        text:               qsTr("Throttle")
                    }
                    AxisMonitor {
                        id:                 throttleAxis
                        height:             ScreenTools.defaultFontPixelHeight
                        width:              axisMonitorWidth
                        mapped:             controller.throttleAxisMapped
                        reversed:           controller.throttleAxisReversed
                    }

                    Connections {
                        target:             _activeJoystick
                        onAxisValues: {
                            rollAxis.axisValue      = roll  * 32768.0
                            pitchAxis.axisValue     = pitch * 32768.0
                            yawAxis.axisValue       = yaw   * 32768.0
                            throttleAxis.axisValue  = _activeJoystick.negativeThrust ? throttle * -32768.0 : (-2 * throttle + 1) * 32768.0
                        }
                    }
                }
            }
            Rectangle {
                color:              Qt.rgba(0,0,0,0)
                border.color:       qgcPal.text
                border.width:       1
                radius:             ScreenTools.defaultFontPixelWidth * 0.5
                width:              axisRect.width
                height:             axisRect.height
                Flow {
                    width:              ScreenTools.defaultFontPixelWidth * 30
                    spacing:            -1
                    anchors.centerIn:   parent
                    Connections {
                        target:     _activeJoystick
                        onRawButtonPressedChanged: (index, pressed) => {
                            if (buttonMonitorRepeater.itemAt(index)) {
                                buttonMonitorRepeater.itemAt(index).pressed = pressed
                            }
                        }
                    }
                    Repeater {
                        id:         buttonMonitorRepeater
                        model:      _activeJoystick ? _activeJoystick.totalButtonCount : []
                        Rectangle {
                            width:          ScreenTools.defaultFontPixelHeight * 1.5
                            height:         width
                            border.width:   1
                            border.color:   qgcPal.text
                            color:          pressed ? qgcPal.buttonHighlight : qgcPal.windowShade
                            property bool pressed
                            QGCLabel {
                                anchors.fill:           parent
                                color:                  pressed ? qgcPal.buttonHighlightText : qgcPal.buttonText
                                horizontalAlignment:    Text.AlignHCenter
                                verticalAlignment:      Text.AlignVCenter
                                text:                   modelData
                            }
                        }
                    }
                }
            }
        }
    }
}


