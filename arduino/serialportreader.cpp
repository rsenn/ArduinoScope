#include "serialportreader.h"

#include <QCoreApplication>
#include <QDebug>

QT_USE_NAMESPACE

SerialPortReader::SerialPortReader(QSerialPort *serial_port, QObject *parent) :
        QObject(parent),
        serial_port_(serial_port)
{
  connect(serial_port, SIGNAL(readyRead()), SLOT(readData()));
  connect(serial_port, SIGNAL(error(QSerialPort::SerialPortError)), SLOT(handleError(QSerialPort::SerialPortError)));
}

SerialPortReader::~SerialPortReader()
{
}

void SerialPortReader::readData()
{

  if (serial_port_->canReadLine())
  {
    data_read_.append(serial_port_->readLine());
  }
  else
  {
    if (!data_read_.isEmpty())
    {
      QDateTime now(QDateTime::currentDateTime());
      emit newLineFetched(data_read_, now);
      data_read_.clear();
    }
  }
}

void SerialPortReader::handleError(QSerialPort::SerialPortError serial_port_rrror)
{
  if (serial_port_rrror == QSerialPort::ReadError)
  {
    qDebug() << "An I/O error occurred while reading the data from port" << serial_port_->portName() << ", error:"
        << serial_port_->errorString();
    while (1)
    {
    } // FIXME: Emit signal to exit
  }
}
