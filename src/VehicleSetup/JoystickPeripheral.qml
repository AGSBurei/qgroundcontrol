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
                                }
                                if(!checked)
                                {
                                    joystickManager.disablePeripheralName = joystickManager.joystickNames[index]
                                    var lst = joystickManager.activePeripherals
                                    if(lst){
                                        for(var i = 0; i < lst.length; i++){
                                            console.log(lst[i]+" :is set active")
                                        }
                                    }
                                }
                            }
                            Component.onCompleted: {
                                var list = joystickManager.activePeripherals
                                if(list.length >0){
                                    for(var i = 0; i< list.length; i++){
                                        if(list[i].name === joystickManager.joystickNames[index])
                                        {
                                            peripheralStatus.checked = true;
                                            console.log(list[i]+" :is set active")
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
