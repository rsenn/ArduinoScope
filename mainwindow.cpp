#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "ui_parameters.h"

#include "arduino/arduinoconnectiondlg.h"
#include <iostream>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent), ui_(new Ui::MainWindow), settings_("ArduinoScope", "ArduinoScope"), is_recording_(false),
      auto_write_csv_(true),
      // serial_port_(new QSerialPort), // FIXME Check this works
      serial_port_reader_(new SerialPortReader(&serial_port_)) {
  ui_->setupUi(this);
  QApplication::setWindowIcon(QIcon("://icons/application-icon.png"));

#ifdef Q_OS_WIN
  ui_->actionRecord->setIcon(QIcon(":/icons/media-record.png"));
  ui_->actionScreenshot->setIcon(QIcon(":/icons/camera-photo.png"));
  ui_->actionSavePath->setIcon(QIcon(":/icons/document-save.png"));
  ui_->actionParameters->setIcon(QIcon(":/icons/preferences-system.png"));
  ui_->actionAbout->setIcon(QIcon(":/icons/help.png"));
#endif

  load(); // Load stored config defaults

  if(!initArduino())
    exit(-1);
}

MainWindow::~MainWindow() {
  if(is_recording_)
    on_actionRecord_triggered();

  save();
  settings_.setValue("window_geometry", saveGeometry());
  settings_.setValue("window_state", saveState());
  settings_.setValue("splitter", ui_->splitter->saveState());
  settings_.sync();
  delete ui_;
}

void
MainWindow::clearLayout(QLayout* layout, bool delete_widgets) {
  while(QLayoutItem* item = layout->takeAt(0)) {
    if(delete_widgets) {
      if(QWidget* widget = item->widget())
        delete widget;
    }
    if(QLayout* childLayout = item->layout())
      clearLayout(childLayout, delete_widgets);
    delete item;
  }
}

void
MainWindow::load() {
  ui_->splitter->restoreState(settings_.value("splitter").toByteArray());

  QString project_dir;
  project_dir = settings_.value("project_directory", "").toString();
  if(!project_dir.isEmpty())
    project_directory_ = project_dir;

  bool tmp;
  tmp = settings_.value("comma_separator", false).toBool();
  data_handler_.comma_decimal_separator_ = tmp;

  tmp = settings_.value("auto_write_csv", true).toBool();
  auto_write_csv_ = tmp;
  ui_->actionSavePath->setEnabled(auto_write_csv_);

  int data_count;
  data_count = settings_.value("data_count", 0).toInt();

  // Retrieve configuration plots data:
  data_handler_.clearTimeAndSensorDataVectors();
  ticks_.clear();
  int i = 0;
  while(i < data_count) {
    QString label;
    label = settings_.value("graph_" + QString::number(i) + "/label", "").toString();
    if(label.isEmpty())
      break;

    QString unit;
    unit = settings_.value("graph_" + QString::number(i) + "/unit", "").toString();

    if(!data_handler_.addSensorData(label, unit))
      qDebug() << "load: could not add sensor data!";
    i++;
  }

  if(data_handler_.getSensorDataSize() == 0) // First launch without a config file
  {
    data_handler_.addSensorData(QString("Blank 0"), QString(""));
    qDebug() << "No data yet, adding blank data";
  }

  i = 0;
  while(i < data_handler_.getSensorDataSize()) {
    bool ticks;
    ticks = settings_.value("graph_" + QString::number(i) + "/ticks", false).toBool();
    ticks_.push_back(ticks);
    i++;
  }

  // Draw graphs
  setupGraphs(ui_->customPlot);
}

void
MainWindow::save() {
  settings_.setValue("comma_separator", data_handler_.comma_decimal_separator_);
  settings_.setValue("auto_write_csv", auto_write_csv_);
  settings_.setValue("data_count", data_handler_.getSensorDataSize());

  unsigned i(0);
  const SensorDataHandler::SensorDataVector& data_vec(data_handler_.getSensorDataVector());

  for(SensorDataHandler::SensorData data : data_vec) {
    settings_.setValue("graph_" + QString::number(i) + "/label", data.label);
    settings_.setValue("graph_" + QString::number(i) + "/unit", data.unit);
    i++;
  }

  for(int i(0); i < ticks_.size(); ++i) settings_.setValue("graph_" + QString::number(i) + "/ticks", ticks_[i]);

  settings_.sync();
}

bool
MainWindow::initArduino() {
  ArduinoConnectionDlg arduinoDlg;
  arduinoDlg.setModal(true);
  arduinoDlg.show();
  if(arduinoDlg.exec() != QDialog::Accepted)
    return false;

  if(arduinoDlg.port_.isEmpty() || arduinoDlg.baudrate_ < 0)
    return false;

  serial_port_.setPortName(arduinoDlg.port_);
  serial_port_.setBaudRate(arduinoDlg.baudrate_);

  if(!serial_port_.open(QIODevice::ReadOnly)) {
    qDebug() << "initArduino: Failed to open port " << serial_port_.portName()
             << "error: " << serial_port_.errorString();
    return false;
  }

  connect(serial_port_reader_, SIGNAL(newLineFetched(QString, QDateTime)), SLOT(newData(QString, QDateTime)));

  ui_->statusBar->showMessage("Connected to " + serial_port_.portName() + " at " +
                                  QString::number(arduinoDlg.baudrate_) + " bauds",
                              6000);
  return true;
}

void
MainWindow::newData(QString line, QDateTime date) {
  QRegExp rx("[;,\\s]");
  QStringList list = line.split(rx, QString::SkipEmptyParts);
  /*if(list.size() != data_handler_.getSensorDataSize()) {
    ui_->statusBar->showMessage("Wrong Arduino message! " + date.toString("hh:mm:ss:zzz"), 1000);
    std::cout << "Wrong Arduino message: " << line.toStdString() << std::endl;

    return;
  }*/

  QVector<double> sensor_values;
  for(QString value_str : list) sensor_values << value_str.toDouble();

  if(sensor_values.size() < data_handler_.getSensorDataSize()) {
    ui_->statusBar->showMessage("Wrong Arduino message! " + date.toString("hh:mm:ss:zzz"), 1000);
    std::cout << "Wrong Arduino message: require " << data_handler_.getSensorDataSize() << " fields - got " <<   sensor_values.size() << std::endl;
    return;
  }

  ui_->statusBar->showMessage("Last Arduino message received at " + date.toString("hh:mm:ss:zzz"), 1000);

  // Update LCDs values
  for(int i(0); i < sensor_values.size() && i < data_handler_.getSensorDataSize(); ++i) {
    QLCDNumber* lcd = ui_->groupBoxSensors->findChild<QLCDNumber*>("lcd_" + QString::number(i));
    if(!lcd) {
        qDebug() << "Could not find LCD lcd_" + QString::number(i);
        continue;
    }

    lcd->display(QString("%1").arg(sensor_values[i], 0, 'f', 2));
  }

  if(!is_recording_)
    return;

  if(!data_handler_.pushBackTimedSensorDataValues(date, sensor_values)) {
    qDebug() << "newData: dataHandler->pushNewData failed!";
    return;
  }

  if(ui_->customPlot->graphCount() != data_handler_.getSensorDataSize())
    return;

  if(data_handler_.getTimeVectorSize() == 0)
    return;

  // Update graph
  const SensorDataHandler::TimeVector& time_vec(data_handler_.getTimeVector());

  // FIXME: Replace by something less computing expensive
  QVector<double> time;
  double base_time(time_vec.front().toMSecsSinceEpoch() / 1000.0);

  for(QDateTime date_time : time_vec) time.push_back((date_time.toMSecsSinceEpoch() / 1000.0) - base_time);

  for(int i(0); i < ui_->customPlot->graphCount(); ++i) {
    const SensorDataHandler::SensorData* tmp;
    tmp = data_handler_.getSensorData(data_handler_.getSensorDataLabels().at(i));
    if(!tmp) {
      qDebug() << "newData: dataHandler->getLabelUnitData failed!" << data_handler_.getSensorDataLabels().at(i);
      return;
    }
    ui_->customPlot->graph(i)->setData(time, tmp->values);
  }

  ui_->customPlot->rescaleAxes(true);
  ui_->customPlot->replot();
}

void
MainWindow::setupGraphs(QCustomPlot* custom_plot) {
  // Do not delete all graphs if updating graph properties
  if(data_handler_.getSensorDataSize() != custom_plot->graphCount()) {
    // Clear graphs and add again
    custom_plot->clearGraphs();
    for(int i(0); i < data_handler_.getSensorDataSize(); ++i) custom_plot->addGraph();
  }

  QStringList labels(data_handler_.getSensorDataLabels());
  QStringList units(data_handler_.getSensorDataUnits());

  for(int i(0); i < data_handler_.getSensorDataSize(); ++i) {
    QString legend(labels.at(i));
    if(units.at(i).size() != 0)
      legend.append(" (" + units.at(i) + ")");
    custom_plot->graph(i)->setName(legend);
  }

  for(int i(0); i < ticks_.size(); ++i) {
    if(!custom_plot->graph(i))
      continue;

    setGraphStyle(custom_plot, i, ticks_[i]);
  }

  custom_plot->legend->setVisible(true);
  custom_plot->xAxis->setLabel("Time (sec)");
  custom_plot->replot();
  setupLCDs();
}

void
MainWindow::setGraphStyle(QCustomPlot* custom_plot, const int id, const bool with_ticks) {
  if(id > custom_plot->graphCount() - 1)
    return;

  switch(id % 8) {
    case 0:
      custom_plot->graph(id)->setPen(QPen(Qt::darkYellow));
      custom_plot->graph(id)->setScatterStyle(with_ticks ? QCPScatterStyle::ssTriangle : QCPScatterStyle::ssNone);
      break;
    case 1:
      custom_plot->graph(id)->setPen(QPen(Qt::red));
      custom_plot->graph(id)->setScatterStyle(with_ticks ? QCPScatterStyle::ssSquare : QCPScatterStyle::ssNone);
      break;
    case 2:
      custom_plot->graph(id)->setPen(QPen(Qt::green));
      custom_plot->graph(id)->setScatterStyle(with_ticks ? QCPScatterStyle::ssPlus : QCPScatterStyle::ssNone);
      break;
    case 3:
      custom_plot->graph(id)->setPen(QPen(Qt::blue));
      custom_plot->graph(id)->setScatterStyle(with_ticks ? QCPScatterStyle::ssDiamond : QCPScatterStyle::ssNone);
      break;
    case 4:
      custom_plot->graph(id)->setPen(QPen(Qt::cyan));
      custom_plot->graph(id)->setScatterStyle(with_ticks ? QCPScatterStyle::ssStar : QCPScatterStyle::ssNone);
      break;
    case 5:
      custom_plot->graph(id)->setPen(QPen(Qt::magenta));
      custom_plot->graph(id)->setScatterStyle(with_ticks ? QCPScatterStyle::ssCrossSquare : QCPScatterStyle::ssNone);
      break;
    case 6:
      custom_plot->graph(id)->setPen(QPen(Qt::black));
      custom_plot->graph(id)->setScatterStyle(with_ticks ? QCPScatterStyle::ssCross : QCPScatterStyle::ssNone);
      break;
    case 7:
      custom_plot->graph(id)->setPen(QPen(Qt::yellow));
      custom_plot->graph(id)->setScatterStyle(with_ticks ? QCPScatterStyle::ssPeace : QCPScatterStyle::ssNone);
      break;
    case 8:
      custom_plot->graph(id)->setPen(QPen(Qt::darkGreen));
      custom_plot->graph(id)->setScatterStyle(with_ticks ? QCPScatterStyle::ssDisc : QCPScatterStyle::ssNone);
      break;
    default:
      custom_plot->graph(id)->setPen(QPen(Qt::gray));
      custom_plot->graph(id)->setScatterStyle(with_ticks ? QCPScatterStyle::ssDot : QCPScatterStyle::ssNone);
      break;
  }
}

void
MainWindow::setupLCDs() {
  // Remove all LCDs
  clearLayout(ui_->LCDsLayout->layout(), true);

  // Add again
  QGridLayout* lcd_grid(new QGridLayout);
  unsigned i(0);

  const SensorDataHandler::SensorDataVector& data_vec(data_handler_.getSensorDataVector());
  for(SensorDataHandler::SensorData data : data_vec) {
    QString lcd_text(data.label);
    if(data.unit.size() != 0)
      lcd_text.append(" (" + data.unit + ")");

    QLabel* lcd_label(new QLabel(lcd_text));
    lcd_label->setMinimumHeight(40);
    QLCDNumber* lcd(new QLCDNumber);
    lcd->setObjectName("lcd_" + QString::number(i));
    lcd_grid->addWidget(lcd_label, i, 0);
    lcd_grid->addWidget(lcd, i, 1);
    i++;
  }

  ui_->LCDsLayout->addLayout(lcd_grid);
  ui_->LCDsLayout->addStretch(1);
}

void
MainWindow::on_actionRecord_triggered() {
  if(!is_recording_) {
    data_handler_.clearTimeVectorAndSensorDataValues();
    is_recording_ = true;
    ui_->actionRecord->setChecked(true);
    ui_->actionParameters->setEnabled(false);
    ui_->actionSavePath->setEnabled(false);
    ui_->groupBoxSensors->setEnabled(true);

    ui_->customPlot->setInteraction(QCP::iRangeZoom, false);
    ui_->customPlot->setInteraction(QCP::iRangeDrag, false);
    ui_->statusBar->showMessage("Started recording", 4000);
    return;
  }

  is_recording_ = false;
  ui_->actionRecord->setChecked(false);
  ui_->actionParameters->setEnabled(true);
  if(auto_write_csv_)
    ui_->actionSavePath->setEnabled(true);
  ui_->groupBoxSensors->setEnabled(false);
  ui_->customPlot->setInteractions(QCP::iRangeZoom | QCP::iRangeDrag);
  ui_->statusBar->showMessage("Stopped recording", 4000);

  if(!auto_write_csv_)
    return;

  if(data_handler_.getTimeVectorSize() == 0) {
    ui_->statusBar->showMessage("No data has been received, skipping file save", 4000);
    return;
  }

  QString date_time(data_handler_.getTimeVector()[0].toString("dd-MM-yyyy hh-mm-ss"));
  if(!data_handler_.writeCSVFile(project_directory_ + "/" + date_time + ".csv")) {
    while(!on_actionSavePath_triggered()) {
    }
  }

  ui_->statusBar->showMessage(date_time + ".csv saved", 4000);
}

void
MainWindow::on_actionScreenshot_triggered() {
  QFileDialog file_dial;
  file_dial.setAcceptMode(QFileDialog::AcceptSave);
  QString file_name =
      file_dial.getSaveFileName(this, tr("Save File"), project_directory_ + "/screen.png", tr("Images (*.png)"));

  if(file_name.size() == 0)
    return;

  // TODO: Users should choose the screenshot size!
  int size_x(1000), size_y(600);

  QFont font;
  font.setPointSize(14);
  ui_->customPlot->xAxis->setTickLabelFont(font);
  ui_->customPlot->yAxis->setTickLabelFont(font);
  ui_->customPlot->replot();

  ui_->customPlot->savePng(file_name, size_x, size_y);

  font.setPointSize(11);
  ui_->customPlot->xAxis->setTickLabelFont(font);
  ui_->customPlot->yAxis->setTickLabelFont(font);
  ui_->customPlot->replot();

  ui_->statusBar->showMessage("Saved screenshot in " + project_directory_, 6000);
}

void
MainWindow::on_actionParameters_triggered() {
  if(is_recording_)
    return;

  QDialog* parameters(new QDialog(0, 0));
  Ui_Dialog parameters_ui;
  parameters_ui.setupUi(parameters);
  parameters_ui.autoWriteCSVcheckBox->setChecked(auto_write_csv_);
  parameters_ui.decimalSep->setCurrentIndex(!data_handler_.comma_decimal_separator_);
  parameters_ui.plots->setValue(data_handler_.getSensorDataSize());

  parameters->setModal(true);
  parameters->show();

  if(parameters->exec() == QDialog::Accepted) {
    auto_write_csv_ = parameters_ui.autoWriteCSVcheckBox->isChecked();
    ui_->actionSavePath->setEnabled(auto_write_csv_);
    data_handler_.comma_decimal_separator_ = !parameters_ui.decimalSep->currentIndex();
    editLegends(parameters_ui.plots->value());
    save();
  }
}

void
MainWindow::editLegends(const int desired_graphs_count) {
  // Shrink data vector if needed, add data if needed
  ticks_.resize(desired_graphs_count);

  if(desired_graphs_count < data_handler_.getSensorDataSize())
    data_handler_.shrinkSensorDataVector(desired_graphs_count);
  else if(desired_graphs_count > data_handler_.getSensorDataSize()) {
    int data_to_be_added = desired_graphs_count - data_handler_.getSensorDataSize();
    unsigned j(0);
    while(data_to_be_added != 0) {
      while(!data_handler_.addSensorData("Blank_" + QString::number(data_handler_.getSensorDataSize() + j), "")) j++;
      data_to_be_added--;
    }
  }

  if(desired_graphs_count != data_handler_.getSensorDataSize()) {
    qDebug() << "editLegends: not the same data count! Abort";
    return;
  }

  // Create GUI
  QDialog* legends(new QDialog(0, 0));
  legends->setModal(true);
  legends->setWindowTitle("Graphs properties");

  QVBoxLayout* vertical_layout(new QVBoxLayout(legends));
  QGridLayout* legends_layout(new QGridLayout);
  QLabel* label(new QLabel("Label:"));
  QLabel* unit(new QLabel("Unit:"));
  QLabel* ticks(new QLabel("Ticks:"));
  legends_layout->addWidget(label, 0, 0);
  legends_layout->addWidget(unit, 0, 1);
  legends_layout->addWidget(ticks, 0, 2);

  const SensorDataHandler::SensorDataVector& data_vec(data_handler_.getSensorDataVector());

  int i;
  for(i = 0; i < data_vec.size(); ++i) {
    SensorDataHandler::SensorData data(data_vec.at(i));
    QLineEdit* label(new QLineEdit(data.label));
    label->setObjectName("label_" + QString::number(i));
    QLineEdit* unit(new QLineEdit(data.unit));
    unit->setObjectName("unit_" + QString::number(i));
    QCheckBox* ticks(new QCheckBox);
    ticks->setChecked(ticks_[i]);

    ticks->setObjectName("ticks_" + QString::number(i));
    legends_layout->addWidget(label, i + 1, 0); // First row has labels
    legends_layout->addWidget(unit, i + 1, 1);
    legends_layout->addWidget(ticks, i + 1, 2);
  }

  vertical_layout->addLayout(legends_layout);
  vertical_layout->addStretch(1);

  QDialogButtonBox* buttons(new QDialogButtonBox);
  vertical_layout->addWidget(buttons);
  buttons->setObjectName(QStringLiteral("legends"));
  buttons->setOrientation(Qt::Horizontal);
  buttons->setStandardButtons(QDialogButtonBox::Cancel | QDialogButtonBox::Ok);

  QObject::connect(buttons, SIGNAL(accepted()), legends, SLOT(accept()));
  QObject::connect(buttons, SIGNAL(rejected()), legends, SLOT(reject()));

  legends->show();

  if(legends->exec() != QDialog::Accepted)
    return;

  // Delete data if needed
  if(data_vec.size() > desired_graphs_count)
    data_handler_.shrinkSensorDataVector(desired_graphs_count);

  // Rename existing data
  for(i = 0; i < data_handler_.getSensorDataSize(); ++i) {
    // Find objects
    QLineEdit* label = legends->findChild<QLineEdit*>("label_" + QString::number(i));
    if(!label)
      return;

    if(label->text().isEmpty()) {
      QMessageBox msg_box;
      msg_box.setWindowTitle("Empty label");
      msg_box.setIcon(QMessageBox::Warning);
      msg_box.setText("The label name cannot be empty!\n"
                      "Setting value to Graph " +
                      QString::number(i));
      msg_box.setStandardButtons(QMessageBox::Ok);
      msg_box.exec();
      label->setText("Graph " + QString::number(i));
    }

    QLineEdit* unit = legends->findChild<QLineEdit*>("unit_" + QString::number(i));
    if(!unit)
      return;

    QCheckBox* ticks = legends->findChild<QCheckBox*>("ticks_" + QString::number(i));
    if(!ticks)
      return;

    ticks_[i] = ticks->isChecked();

    if(!data_handler_.renameSensorData(i, label->text())) // Rename fails if label name is already taken
    {
      QMessageBox msg_box;
      msg_box.setWindowTitle("Cannot rename label");
      msg_box.setIcon(QMessageBox::Warning);
      msg_box.setText("The label name is already used for another graph!\n"
                      "A default name will be used");
      msg_box.setStandardButtons(QMessageBox::Ok);
      msg_box.exec();
      unsigned j(i);
      while(!data_handler_.renameSensorData(i, "Graph " + QString::number(j++))) { // Find a free name
      };
    }
    data_handler_.renameSensorDataUnit(i, unit->text());
  }

  setupGraphs(ui_->customPlot);

  // Set graph style AFTER setupGraph to set the ticks
  if(data_handler_.getSensorDataSize() != ticks_.size()) {
    save();
    return;
  }

  for(int(i) = 0; i < data_handler_.getSensorDataSize(); ++i) setGraphStyle(ui_->customPlot, ticks_[i], false);

  save();
}

bool
MainWindow::on_actionSavePath_triggered() {
  // Not a problem is project_directory_ is empty, defaults to the binary directory
  QString new_directory = QFileDialog::getExistingDirectory(this,
                                                            tr("Choose save folder for data in CSV format"),
                                                            project_directory_,
                                                            QFileDialog::ShowDirsOnly);

  if(new_directory.isEmpty())
    return false;

  project_directory_ = new_directory;
  settings_.setValue("project_directory", project_directory_);
  settings_.sync();
  ui_->statusBar->showMessage("New saving directory: " + project_directory_, 4000);
  return true;
}

void
MainWindow::on_actionAbout_triggered() {
  QDialog* about(new QDialog(0, 0));
  about->setModal(true);
  about->setWindowTitle("About");

  QVBoxLayout* vertical_layout(new QVBoxLayout(about));
  QLabel* label_soft(new QLabel("Software version: " + QString(VERSION)));
  QLabel* label_link(new QLabel(
      "Repository: <a "
      "href=\"https://github.com/VictorLamoine/ArduinoScope\">https://github.com/VictorLamoine/ArduinoScope</a>"));
  QLabel* label_author(
      new QLabel("Authors: Victor Lamoine, <a "
                 "href=\"https://github.com/VictorLamoine/ArduinoScope/graphs/contributors\">contributors</a>"));
  QLabel* label_license(new QLabel(
      "License: <a href=\"https://github.com/VictorLamoine/ArduinoScope/blob/master/LICENSE.txt\">BSD-3-Clause</a>, "
      "QCustomPlot license: GPL v3"));
  vertical_layout->addWidget(label_soft);
  vertical_layout->addWidget(label_link);
  vertical_layout->addWidget(label_author);
  vertical_layout->addWidget(label_license);
  vertical_layout->addStretch(1);
  about->show();
}
