#include "arduinoconnectiondlg.h"
#include "ui_arduinoconnectiondlg.h"

#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include <QPushButton>

ArduinoConnectionDlg::ArduinoConnectionDlg(QWidget *parent) :
        QDialog(parent, Qt::WindowTitleHint | Qt::WindowSystemMenuHint),
        port_(""),
        baudrate_(-1),
        ui_(new Ui::ArduinoConnectionDlg)
{
  ui_->setupUi(this);
  connect(ui_->buttonBox, SIGNAL(accepted(void)), this, SLOT(accept()));
  connect(ui_->buttonBox, SIGNAL(rejected(void)), this, SLOT(reject()));
  addBaudRates();
  addAvailablePorts();
}

ArduinoConnectionDlg::~ArduinoConnectionDlg()
{
  delete ui_;
}

void ArduinoConnectionDlg::addBaudRates()
{
  ui_->baudrateCbx->addItem("1200");
  ui_->baudrateCbx->addItem("2400");
  ui_->baudrateCbx->addItem("4800");
  ui_->baudrateCbx->addItem("9600");
  ui_->baudrateCbx->addItem("19200");
  ui_->baudrateCbx->addItem("38400");
  ui_->baudrateCbx->addItem("57600");
  ui_->baudrateCbx->addItem("115200");
  ui_->baudrateCbx->setCurrentIndex(6);
}

void ArduinoConnectionDlg::on_baudrateCbx_currentIndexChanged(int index)
{
  switch (index)
  {
    case 0:
      baudrate_ = QSerialPort::Baud1200;
      break;
    case 1:
      baudrate_ = QSerialPort::Baud2400;
      break;
    case 2:
      baudrate_ = QSerialPort::Baud4800;
      break;
    case 3:
      baudrate_ = QSerialPort::Baud9600;
      break;
    case 4:
      baudrate_ = QSerialPort::Baud19200;
      break;
    case 5:
      baudrate_ = QSerialPort::Baud38400;
      break;
    case 6:
      baudrate_ = QSerialPort::Baud57600;
      break;
    case 7:
      baudrate_ = QSerialPort::Baud115200;
      break;
    default:
      baudrate_ = QSerialPort::Baud115200;
      break;
  }
}

void ArduinoConnectionDlg::addAvailablePorts()
{
  foreach(const QSerialPortInfo &info, QSerialPortInfo::availablePorts())ui_->portCbx->addItem(info.portName());

  if (ui_->portCbx->count() == 0)
  ui_->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
}

void ArduinoConnectionDlg::on_portCbx_currentIndexChanged(const QString &arg1)
{
  port_ = arg1;
}
