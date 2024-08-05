#pragma once

#include <QtCore/QString>
#include <QtCore/QByteArray>
#include <QtCore/QLoggingCategory>

#include <qserialport.h>
#include <qserialportinfo.h>

#include <jni.h>

Q_DECLARE_LOGGING_CATEGORY(AndroidSerialLog);

class QSerialPortPrivate;

namespace AndroidSerial
{
    enum DataBits {
        Data5 = 5,
        Data6 = 6,
        Data7 = 7,
        Data8 = 8
    };

    enum Parity {
        NoParity = 0,
        OddParity,
        EvenParity,
        MarkParity,
        SpaceParity
    };

    enum StopBits {
        OneStop = 1,
        OneAndHalfStop = 3,
        TwoStop = 2
    };

    enum ControlLine {
        RtsControlLine = 0,
        CtsControlLine,
        DtrControlLine,
        DsrControlLine,
        CdControlLine,
        RiControlLine
    };

    enum FlowControl {
        NoFlowControl = 0,
        RtsCtsFlowControl,
        DtrDsrFlowControl,
        XonXoffFlowControl,
        XonXoffInlineFlowControl
    };

    static constexpr char CHAR_XON = 17;
    static constexpr char CHAR_XOFF = 19;

    void setNativeMethods();
    void jniDeviceHasDisconnected(JNIEnv *env, jobject obj, jlong userData);
    void jniDeviceNewData(JNIEnv *env, jobject obj, jlong userData, jbyteArray data);
    void jniDeviceException(JNIEnv *env, jobject obj, jlong userData, jstring message);
    QList<QSerialPortInfo> availableDevices();
    QString getSerialNumber(int deviceId);
    int getDeviceId(const QString &portName);
    int getDeviceHandle(int deviceId);
    int open(const QString &portName, QSerialPortPrivate *userData);
    bool close(int deviceId);
    bool isOpen(const QString &portName);
    QByteArray read(int deviceId, int length, int timeout);
    int write(int deviceId, QByteArrayView data, int length, int timeout, bool async);
    bool setParameters(int deviceId, int baudRate, int dataBits, int stopBits, int parity);
    bool getCarrierDetect(int deviceId);
    bool getClearToSend(int deviceId);
    bool getDataSetReady(int deviceId);
    bool getDataTerminalReady(int deviceId);
    bool setDataTerminalReady(int deviceId, bool set);
    bool getRingIndicator(int deviceId);
    bool getRequestToSend(int deviceId);
    bool setRequestToSend(int deviceId, bool set);
    QSerialPort::PinoutSignals getControlLines(int deviceId);
    int getFlowControl(int deviceId);
    bool setFlowControl(int deviceId, int flowControl);
    bool purgeBuffers(int deviceId, bool input, bool output);
    bool setBreak(int deviceId, bool set);
    bool startReadThread(int deviceId);
    bool stopReadThread(int deviceId);
    bool readThreadRunning(int deviceId);
    int getReadBufferSize(int deviceId);
}; // namespace AndroidSerial
