#ifndef SERIALPORTREADER_H
#define SERIALPORTREADER_H

#include <QtSerialPort/QSerialPort>
#include <QTextStream>
#include <QTimer>
#include <QDateTime>
#include <QByteArray>
#include <QObject>

QT_USE_NAMESPACE

QT_BEGIN_NAMESPACE

QT_END_NAMESPACE

class SerialPortReader : public QObject
{
Q_OBJECT

public:
  SerialPortReader(QSerialPort *serial_port, QObject *parent = 0);
  ~SerialPortReader();

signals:
  void newLineFetched(QString line, QDateTime data_time);

private slots:
  void readData();
  void handleError(QSerialPort::SerialPortError error);

private:
  QSerialPort *serial_port_;
  QByteArray data_read_;
};

#endif
