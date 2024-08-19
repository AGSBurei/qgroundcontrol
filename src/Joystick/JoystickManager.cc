/****************************************************************************
 *
 * (c) 2009-2020 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/


#include "JoystickManager.h"
#include "MultiVehicleManager.h"
#include "Joystick.h"
#if !defined(__mobile__) || defined(QGC_SDL_JOYSTICK)
    #include "JoystickSDL.h"
    #define __sdljoystick__
#elif defined(Q_OS_ANDROID)
    #include "JoystickAndroid.h"
#endif
#include "QGCLoggingCategory.h"

#include <QtCore/QSettings>
#include <QtQml/QQmlEngine>
#include <QtQml/QtQml>

QGC_LOGGING_CATEGORY(JoystickManagerLog, "JoystickManagerLog")

JoystickManager::JoystickManager(QGCApplication* app, QGCToolbox* toolbox)
    : QGCTool(app, toolbox)
    , _activeJoystick(nullptr)
    , _activePeripheralsList()
    , _multiVehicleManager(nullptr)
{
    // qCDebug(JoystickManagerLog) << Q_FUNC_INFO << this;
}

JoystickManager::~JoystickManager()
{
    QMap<QString, Joystick*>::iterator i;
    for (i = _name2JoystickMap.begin(); i != _name2JoystickMap.end(); ++i) {
        qCDebug(JoystickManagerLog) << "Releasing joystick:" << i.key();
        i.value()->stop();
        delete i.value();
    }

    // qCDebug(JoystickManagerLog) << Q_FUNC_INFO << this;
}

void JoystickManager::setToolbox(QGCToolbox *toolbox)
{
    QGCTool::setToolbox(toolbox);

    _multiVehicleManager = _toolbox->multiVehicleManager();

    QQmlEngine::setObjectOwnership(this, QQmlEngine::CppOwnership);
    qmlRegisterUncreatableType<JoystickManager>("QGroundControl.JoystickManager", 1, 0, "JoystickManager", "Reference only");
    qmlRegisterUncreatableType<Joystick>       ("QGroundControl.JoystickManager", 1, 0, "Joystick",        "Reference only");
}

void JoystickManager::init() {
#ifdef __sdljoystick__
    if (!JoystickSDL::init()) {
        return;
    }
    _setActiveJoystickFromSettings();
#elif defined(Q_OS_ANDROID)
    if (!JoystickAndroid::init(this)) {
        return;
    }
    connect(this, &JoystickManager::updateAvailableJoysticksSignal, this, &JoystickManager::restartJoystickCheckTimer);
#endif
    connect(&_joystickCheckTimer, &QTimer::timeout, this, &JoystickManager::_updateAvailableJoysticks);
    _joystickCheckTimerCounter = 5;
    _joystickCheckTimer.start(1000);
}

void JoystickManager::_setActiveJoystickFromSettings(void)
{
    QMap<QString,Joystick*> newMap;

#ifdef __sdljoystick__
    // Get the latest joystick mapping
    newMap = JoystickSDL::discover(_multiVehicleManager);
#elif defined(Q_OS_ANDROID)
    newMap = JoystickAndroid::discover(_multiVehicleManager);
#endif

    if (_activeJoystick && !newMap.contains(_activeJoystick->name())) {
        qCDebug(JoystickManagerLog) << "Active joystick removed";
        setActiveJoystick(nullptr);
    }

    // Check to see if our current mapping contains any joysticks that are not in the new mapping
    // If so, those joysticks have been unplugged, and need to be cleaned up
    QMap<QString, Joystick*>::iterator i;
    for (i = _name2JoystickMap.begin(); i != _name2JoystickMap.end(); ++i) {
        if (!newMap.contains(i.key())) {
            qCDebug(JoystickManagerLog) << "Releasing joystick:" << i.key();
            i.value()->stopPolling();
            i.value()->wait(1000);
            i.value()->deleteLater();
        }
    }

    _name2JoystickMap = newMap;
    emit availableJoysticksChanged();

    if (!_name2JoystickMap.count()) {
        setActiveJoystick(nullptr);
        return;
    }

    QSettings settings;

    settings.beginGroup(_settingsGroup);
    QString name = settings.value(_settingsKeyActiveJoystick).toString();

    if (name.isEmpty()) {
        name = _name2JoystickMap.first()->name();
    }

    setActiveJoystick(_name2JoystickMap.value(name, _name2JoystickMap.first()));
    settings.setValue(_settingsKeyActiveJoystick, _activeJoystick->name());
}

Joystick* JoystickManager::activeJoystick(void)
{
    return _activeJoystick;
}

QList<Joystick*> JoystickManager::activePeripherals(void)
{
    qDebug()<< _activePeripheralsList;
    return _activePeripheralsList;
}

void JoystickManager::setActiveJoystick(Joystick* joystick)
{
    QSettings settings;

    if (joystick != nullptr && !_name2JoystickMap.contains(joystick->name())) {
        qCWarning(JoystickManagerLog) << "Set active not in map" << joystick->name();
        return;
    }

    if (_activeJoystick == joystick) {
        return;
    }

    if (_activeJoystick) {
        _activeJoystick->stopPolling();
    }

    _activeJoystick = joystick;

    if (_activeJoystick != nullptr) {
        qCDebug(JoystickManagerLog) << "Set active:" << _activeJoystick->name();

        settings.beginGroup(_settingsGroup);
        settings.setValue(_settingsKeyActiveJoystick, _activeJoystick->name());
    }

    emit activeJoystickChanged(_activeJoystick);
    emit activeJoystickNameChanged(_activeJoystick?_activeJoystick->name():"");
}

void JoystickManager::setActivePeripherals(Joystick* joystick)
{

    QSettings settings;
    QList<QString> periphNamesList;

    qDebug() << "current joystick in pipe: " << joystick ;
    if (joystick != nullptr && !_name2JoystickMap.contains(joystick->name())) {
        qCWarning(JoystickManagerLog) << "Set active not in map" << joystick->name();
        return;
    }

    if(_activePeripheralsList.contains(joystick)){
        return;
    }

    if (!_activePeripheralsList.empty()){
        for (int i = 0; i < _activePeripheralsList.length(); i++){
            _activePeripheralsList[i]->stopPolling();
        }
    }
    _activePeripheralsList.append(joystick);
    qDebug() << "list is: " <<_activePeripheralsList.length() ;
    for(int i = 0; i < _activePeripheralsList.length(); i++){
        qDebug() << _activePeripheralsList[i] <<" :is in the list";
    }

    for (int i = 0; i < _activePeripheralsList.length(); i++){
        if(_activePeripheralsList[i] == joystick){
            qCDebug(JoystickManagerLog) << "Set active:" << _activePeripheralsList[i]->name();
            settings.beginGroup(_settingsGroup);
            settings.setValue(_settingsKeyActiveJoystick, _activePeripheralsList[i]->name());
        }
    }

    for(int i = 0; i < _activePeripheralsList.length(); i++){
        periphNamesList.append(_activePeripheralsList[i]->name());
    }
    emit activePeripheralsChanged(_activePeripheralsList);
    emit activePeripheralsNamesChanged(periphNamesList);
}

void JoystickManager::setActivePeripherals(QList<Joystick*> peripherals)
{

    QSettings settings;
    QList<QString> periphNamesList;
    for (int i = 0; i < peripherals.length(); i++){
        qDebug() << "current joystick in pipe: " << peripherals[i] ;
        if (peripherals[i] != nullptr && !_name2JoystickMap.contains(peripherals[i]->name())) {
            qCWarning(JoystickManagerLog) << "Set active not in map" << peripherals[i]->name();
            return;
        }

        if(_activePeripheralsList.contains(peripherals[i])){
            return;
        }

        if (!_activePeripheralsList.empty()){
            for (int i = 0; i < _activePeripheralsList.length(); i++){
                _activePeripheralsList[i]->stopPolling();
            }
        }
        if(!peripherals.empty()){

        }
        _activePeripheralsList.append(peripherals[i]);

        for (int i = 0; i < _activePeripheralsList.length(); i++){
            if(_activePeripheralsList[i] == peripherals[i]){
                qCDebug(JoystickManagerLog) << "Set active:" << _activePeripheralsList[i]->name();
                settings.beginGroup(_settingsGroup);
                settings.setValue(_settingsKeyActiveJoystick, _activePeripheralsList[i]->name());
            }
        }

        periphNamesList.append(peripherals[i]->name());

        emit activePeripheralsChanged(_activePeripheralsList);
        emit activePeripheralsNamesChanged(periphNamesList);
    }
}

void JoystickManager::disablePeripheral(Joystick* joystick) {
    Joystick* peripheral;
    if(_activePeripheralsList.contains(joystick)){
        int i = _activePeripheralsList.indexOf(joystick);
        peripheral = _activePeripheralsList[i];
        qCDebug(JoystickManagerLog) << "removing:" << _activePeripheralsList[i]->name();
        _activePeripheralsList[i]->stopPolling();
        _activePeripheralsList.removeAt(i);

        emit activePeripheralsChanged(_activePeripheralsList);
        emit disablePeripheralNameChanged(peripheral->name());
    }else{
        qCWarning(JoystickManagerLog) << "target for deactivation does not exist";
    }
}

QVariantList JoystickManager::joysticks(void)
{
    QVariantList list;

    for (const QString &name: _name2JoystickMap.keys()) {
        list += QVariant::fromValue(_name2JoystickMap[name]);
    }

    return list;
}

QStringList JoystickManager::joystickNames(void)
{
    return _name2JoystickMap.keys();
}

QString JoystickManager::activeJoystickName(void)
{
    return _activeJoystick ? _activeJoystick->name() : QString();
}

QString JoystickManager::activePeripheralName(void)
{
    return _activeJoystick ? _activeJoystick->name() : QString();
}

bool JoystickManager::setActiveJoystickName(const QString& name)
{
    if (_name2JoystickMap.contains(name)) {
        setActiveJoystick(_name2JoystickMap[name]);
        return true;
    } else {
        qCWarning(JoystickManagerLog) << "Set active not in map" << name;
        return false;
    }
}

bool JoystickManager::setActivePeripheralName(const QString& name)
{
    if (_name2JoystickMap.contains(name)) {
        setActivePeripherals(_name2JoystickMap[name]);
        return true;
    } else {
        qCWarning(JoystickManagerLog) << "Set active not in map" << name;
        return false;
    }
}

bool JoystickManager::disablePeripheralName(const QString& name){
    if (_name2JoystickMap.contains(name)) {
        disablePeripheral(_name2JoystickMap[name]);
        return true;
    } else {
        qCWarning(JoystickManagerLog) << "Disabled not in map" << name;
        return false;
    }
}

/*
 * TODO: move this to the right place: JoystickSDL.cc and JoystickAndroid.cc respectively and call through Joystick.cc
 */
void JoystickManager::_updateAvailableJoysticks()
{
#ifdef __sdljoystick__
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch(event.type) {
        case SDL_QUIT:
            qCDebug(JoystickManagerLog) << "SDL ERROR:" << SDL_GetError();
            break;
        case SDL_JOYDEVICEADDED:
            qCDebug(JoystickManagerLog) << "Joystick added:" << event.jdevice.which;
            _setActiveJoystickFromSettings();
            break;
        case SDL_JOYDEVICEREMOVED:
            qCDebug(JoystickManagerLog) << "Joystick removed:" << event.jdevice.which;
            _setActiveJoystickFromSettings();
            break;
        default:
            break;
        }
    }
#elif defined(Q_OS_ANDROID)
    _joystickCheckTimerCounter--;
    _setActiveJoystickFromSettings();
    if (_joystickCheckTimerCounter <= 0) {
        _joystickCheckTimer.stop();
    }
#endif
}

void JoystickManager::restartJoystickCheckTimer()
{
    _joystickCheckTimerCounter = 5;
    _joystickCheckTimer.start(1000);
}
