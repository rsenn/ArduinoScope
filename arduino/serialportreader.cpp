#include "serialportreader.h"

#include <QCoreApplication>
#include <QDebug>
#include <iostream>

QT_USE_NAMESPACE

SerialPortReader::SerialPortReader(QSerialPort* serial_port, QObject* parent)
    : QObject(parent), serial_port_(serial_port) {
  connect(serial_port, SIGNAL(readyRead()), SLOT(readData()));
  connect(serial_port, SIGNAL(error(QSerialPort::SerialPortError)), SLOT(handleError(QSerialPort::SerialPortError)));
}

SerialPortReader::~SerialPortReader() {}

void
SerialPortReader::readData() {

  if(serial_port_->canReadLine()) {
    QByteArray line = (serial_port_->readLine());
    QString lineStr(line);

    QStringList fields = lineStr.trimmed().split(QRegExp("[^-+0-9.]+"));

    bool ok = std::all_of(fields.cbegin(), fields.cend(), [](const QString& field) -> bool {
      bool success = false;
      field.toDouble(&success);
      return success;
    });

    if(ok) {
      data_read_.append(lineStr);
    }

    std::cout << (ok ? "OK" : "IGNORE") << ": " << lineStr.toStdString() << std::endl;
  }

  if(!data_read_.isEmpty()) {
    QDateTime now(QDateTime::currentDateTime());
    emit newLineFetched(data_read_, now);
    data_read_.clear();
  }
}

void
SerialPortReader::handleError(QSerialPort::SerialPortError serial_port_rrror) {
  if(serial_port_rrror == QSerialPort::ReadError) {
    qDebug() << "An I/O error occurred while reading the data from port" << serial_port_->portName()
             << ", error:" << serial_port_->errorString();
    while(1) {
    } // FIXME: Emit signal to exit
  }
}
