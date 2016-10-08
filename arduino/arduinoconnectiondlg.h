#ifndef ARDUINOCONNECTIONDLG_H
#define ARDUINOCONNECTIONDLG_H

#include <QDialog>
#include <QListWidgetItem>

namespace Ui
{
class ArduinoConnectionDlg;
}

class ArduinoConnectionDlg : public QDialog
{
Q_OBJECT

public:
  explicit
  ArduinoConnectionDlg(QWidget *parent = 0);
  ~ArduinoConnectionDlg();

  QString port_;
  int baudrate_;

private slots:
  void on_baudrateCbx_currentIndexChanged(int index);
  void on_portCbx_currentIndexChanged(const QString &arg1);

private:
  Ui::ArduinoConnectionDlg *ui_;
  void addBaudRates();
  void addAvailablePorts();
};

#endif // ARDUINOCONNECTIONDLG_H
