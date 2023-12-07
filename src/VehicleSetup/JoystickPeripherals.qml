/****************************************************************************
 *
 * (c) 2009-2020 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

import QtQuick                      2.11
import QtQuick.Controls             2.4
import QtQuick.Dialogs              1.3
import QtQuick.Layouts              1.11

import QGroundControl               1.0
import QGroundControl.Palette       1.0
import QGroundControl.Controls      1.0
import QGroundControl.ScreenTools   1.0
import QGroundControl.Controllers   1.0
import QGroundControl.FactSystem    1.0
import QGroundControl.FactControls  1.0

Item {
    width:                  mainCol.width  + (ScreenTools.defaultFontPixelWidth  * 2)
    height:                 mainCol.height + (ScreenTools.defaultFontPixelHeight * 2)
    property var peripheralList: []
    Column {
        id:                 mainCol
        anchors.centerIn:   parent
        spacing:            ScreenTools.defaultFontPixelHeight

        Row{
            QGCLabel {
                id:         joystickPeripheralsPageLabel
                text:       qsTr("Select wanted peripheral to be active")
            }
        }
        GridLayout{
            columns: 2
            columnSpacing:      ScreenTools.defaultFontPixelWidth
            rowSpacing:         ScreenTools.defaultFontPixelHeight
            Repeater{
                model: joystickManager.joystickNames
                Row{
                    Column{
                        QGCLabel {
                            text: joystickManager.joystickNames[index]
                            }
                        }
                    Column{
                         QGCCheckBox {
                            id: peripheralStatus
                            onClicked:{
                                if (checked) {
                                    joystickManager.activePeripheralName = joystickManager.joystickNames[index]
                                    var lst = joystickManager.activePeripherals
                                    if(lst){
                                        for(var i = 0; i < lst.length; i++){
                                            console.log(lst[i]+" :is set active")
                                        }
                                    }
                                }else{
                                }
                            }
                            Component.onCompleted: {
                                var list = joystickManager.activePeripherals
                                if(list.length >0){
                                    for(var i = 0; i< list.length; i++){
                                        if(list[i].name === joystickManager.joystickNames[index])
                                        {
                                            peripheralStatus.checked = true;
                                            console.log(lst[i]+" :is set active")
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

