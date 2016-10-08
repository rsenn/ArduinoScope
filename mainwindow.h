#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "qcustomplot/qcustomplot.h"

#include "version.h"
#include "arduino/serialportreader.h"
#include "sensordatahandler.h"

namespace Ui
{
class MainWindow;
}

class MainWindow : public QMainWindow
{
Q_OBJECT

public:
  explicit
  MainWindow(QWidget *parent = 0);
  ~MainWindow();

private:
  void clearLayout(QLayout* layout, bool deleteWidgets);
  void load();
  void save();
  bool initArduino();
  void setupGraphs(QCustomPlot *custom_plot);
  void setGraphStyle(QCustomPlot* custom_plot, const int id, const bool with_ticks);
  void setupLCDs();
  void editLegends(const int desired_graphs_count);

  Ui::MainWindow *ui_;
  QSettings settings_;

  QString project_directory_;
  bool is_recording_;
  bool auto_write_csv_;

  QSerialPort serial_port_;
  SerialPortReader* serial_port_reader_;
  SensorDataHandler data_handler_;
  QVector<double> ticks_;

private slots:
  void newData(QString line, QDateTime date);
  void on_actionRecord_triggered();
  void on_actionScreenshot_triggered();
  void on_actionParameters_triggered();
  bool on_actionSavePath_triggered();
  void on_actionAbout_triggered();

};

#endif // MAINWINDOW_H
