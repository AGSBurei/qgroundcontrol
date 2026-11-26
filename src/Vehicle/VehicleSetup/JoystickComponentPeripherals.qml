import QtQuick
import QtQuick.Layouts

import QGroundControl
import QGroundControl.Controls

/// Lets the user pick which other connected devices are used alongside the active joystick.
/// Additional peripherals provide button actions only; stick axes come from the active joystick.
QGCGroupBox {
    id: root

    heading: qsTr("Additional Peripherals")
    headingDescription: qsTr("Use other connected devices at the same time as the active joystick. They only provide button actions; sticks are read from the active joystick. To assign a device's buttons, select it as the active joystick first.")

    readonly property string activeJoystickName: joystickManager.activeJoystick ? joystickManager.activeJoystick.name : ""
    readonly property var otherJoystickNames: (joystickManager.availableJoystickNames || []).filter(name => name !== activeJoystickName)
    readonly property var activePeripheralNames: joystickManager.activePeripheralNames || []

    Repeater {
        model: root.otherJoystickNames

        QGCCheckBox {
            required property string modelData

            text: modelData
            checked: root.activePeripheralNames.includes(modelData)

            onClicked: {
                joystickManager.setPeripheralActive(modelData, checked)
                // Clicking breaks the binding; restore it so the box always reflects the manager's state
                checked = Qt.binding(() => root.activePeripheralNames.includes(modelData))
            }
        }
    }
}
