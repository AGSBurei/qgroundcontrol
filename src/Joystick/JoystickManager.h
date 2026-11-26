#pragma once

#include <QtCore/QObject>
#include <QtCore/QTimer>
#include <QtCore/QVariantList>
#include <QtQmlIntegration/QtQmlIntegration>

class Joystick;
class JoystickManagerSettings;
class Vehicle;

class JoystickManager : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("")
    Q_MOC_INCLUDE("Joystick.h")

    /// List of available joystick names
    ///     Note: The active joystick name may be not in this list if the joystick is not currently connected
    Q_PROPERTY(QStringList  availableJoystickNames READ availableJoystickNames NOTIFY availableJoystickNamesChanged)

    Q_PROPERTY(Joystick *activeJoystick READ activeJoystick NOTIFY activeJoystickChanged)
    Q_PROPERTY(bool activeJoystickEnabledForActiveVehicle READ activeJoystickEnabledForActiveVehicle WRITE setActiveJoystickEnabledForActiveVehicle NOTIFY activeJoystickEnabledForActiveVehicleChanged)

    /// Number of connected joysticks
    Q_PROPERTY(int joystickCount READ joystickCount NOTIFY availableJoystickNamesChanged)

    /// Names of the connected devices used alongside the active joystick (button actions only).
    /// Never contains the active joystick itself.
    Q_PROPERTY(QStringList activePeripheralNames READ activePeripheralNames NOTIFY activePeripheralsChanged)

#ifdef QGC_UNITTEST_BUILD
    friend class JoystickManagerTest;
#endif

public:
    explicit JoystickManager(QObject *parent = nullptr);
    ~JoystickManager() override;

    static JoystickManager *instance();

    QStringList availableJoystickNames() const { return _name2JoystickMap.keys(); }
    int joystickCount() const { return _name2JoystickMap.count(); }

    Joystick *activeJoystick();
    bool activeJoystickEnabledForActiveVehicle() const;
    void setActiveJoystickEnabledForActiveVehicle(bool enabled);

    /// Get all joysticks in a linked group
    /// Returns empty list if groupId is empty or no matches found
    Q_INVOKABLE QStringList linkedGroupMembers(const QString &groupId) const;

    /// Get joystick by name (returns nullptr if not found)
    Q_INVOKABLE Joystick *joystickByName(const QString &name) const;

    /// Multi-peripheral support: use other connected devices alongside the active joystick
    QStringList activePeripheralNames() const;
    /// Add or remove a device from the saved peripheral selection. Selecting the active joystick
    /// is allowed: it is used as an additional peripheral again once another joystick becomes active.
    Q_INVOKABLE void setPeripheralActive(const QString &name, bool active);

signals:
    void activeJoystickChanged(Joystick *joystick);
    void activeJoystickEnabledForActiveVehicleChanged();
    void availableJoystickNamesChanged();
    void activePeripheralsChanged();

public slots:
    void init();

private slots:
    /// Checks for added or removed joysticks and updates the internal map accordingly
    void _checkForAddedOrRemovedJoysticks();
    void _activeVehicleChanged(Vehicle *activeVehicle);
    void _setActiveJoystickByName(const QString &name);

    // SDL event handlers (called via QMetaObject::invokeMethod from sdlEventWatcher)
    void _handleUpdateComplete(int instanceId);
    void _handleBatteryUpdated(int instanceId);
    void _handleGamepadRemapped(int instanceId);
    void _handleTouchpadEvent(int instanceId, int touchpad, int finger, bool down, float x, float y, float pressure);
    void _handleSensorUpdate(int instanceId, int sensor, float x, float y, float z);

private:
    void _setActiveJoystickFromSettings();
    void _setActiveJoystick(Joystick *joystick);
    bool _joystickEnabledForVehicle(Vehicle *vehicle) const;
    void _setJoystickEnabledForVehicle(Vehicle *vehicle, bool enabled);
    Joystick *_findJoystickByInstanceId(int instanceId);
    void _updatePollingTimer();

    QStringList _savedPeripheralNames() const;
    /// Bring _activePeripherals in line with the saved selection, the connected devices and the active joystick
    void _updateActivePeripherals();
    void _startPeripheralPolling(Joystick *peripheral);
    void _releasePeripheral(Joystick *peripheral);
    void _restartPeripheralPolling();

    JoystickManagerSettings *_joystickManagerSettings = nullptr;
    Joystick *_activeJoystick = nullptr;
    QList<Joystick*> _activePeripherals;  ///< Additional devices, never includes _activeJoystick
    QMap<QString, Joystick*> _name2JoystickMap;
    QTimer _pollTimer;
};
