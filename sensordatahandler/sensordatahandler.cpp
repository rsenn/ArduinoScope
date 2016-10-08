#include "sensordatahandler.h"

#include <cmath>

#include <QFile>
#include <QTextStream>
#include <QLocale>
#include <QMessageBox>
#include <QDebug>

SensorDataHandler::SensorDataHandler() :
    comma_decimal_separator_(false), data_mutex_(QMutex::RecursionMode::Recursive), csv_separator_(";")
{
}

bool SensorDataHandler::addSensorData(const QString label, const QString unit)
{
  if (label.size() == 0)
    return false;

  data_mutex_.lock();

  // Can't add the same label twice
  for (SensorData data : data_.second)
  {
    if (data.label == label)
    {
      data_mutex_.unlock();
      return false;
    }
  }

  // FIXME: Improve initialization
  SensorData tmp;
  tmp.label = label;
  tmp.unit = unit;
  data_.second.push_back(tmp);
  data_mutex_.unlock();
  return true;
}

bool SensorDataHandler::removeSensorData(const QString label)
{
  data_mutex_.lock();

  // Search for this label and remove it
  bool found(false);
  for (int i(0); i < data_.second.size(); ++i)
  {
    if (data_.second.at(i).label == label)
    {
      data_.second.removeAt(i);
      found = true;
      break;
    }
  }

  data_mutex_.unlock();
  return found;
}

bool SensorDataHandler::removeSensorData(const int index)
{
  data_mutex_.lock();

  if (index - 1 >= data_.second.size())
  {
    data_mutex_.unlock();
    return false;
  }

  data_.second.removeAt(index);
  data_mutex_.unlock();
  return true;
}

bool SensorDataHandler::shrinkSensorDataVector(const int size)
{
  if (size <= 0 || size > data_.second.size())
    return false;

  data_mutex_.lock();
  data_.second.resize(size);
  data_mutex_.unlock();
  return true;
}

void SensorDataHandler::clearTimeAndSensorDataVectors()
{
  data_mutex_.lock();
  // Clear time and sensor data vectors
  data_.first.clear();
  data_.second.clear();
  data_mutex_.unlock();
}

void SensorDataHandler::clearTimeVectorAndSensorDataValues()
{
  data_mutex_.lock();
  // Clear time vector
  data_.first.clear();
  // Clear sensor data values
  for (SensorData data : data_.second)
    data.values.clear();
  data_mutex_.unlock();
}

bool SensorDataHandler::renameSensorData(const QString old_label, const QString new_label)
{
  data_mutex_.lock();

  // If new label is already taken, error!
  for (int i(0); i < data_.second.size(); ++i)
  {
    if (data_.second.at(i).label == new_label)
    {
      data_mutex_.unlock();
      return false;
    }
  }

  // Search for old label and store it's index
  bool found(false);
  int i;
  for (i = 0; i < data_.second.size(); ++i)
  {
    if (data_.second.at(i).label == old_label)
    {
      found = true;
      break;
    }
  }

  if (!found)
  {
    data_mutex_.unlock();
    return false;
  }

  data_.second[i].label = new_label;
  data_mutex_.unlock();
  return true;
}

bool SensorDataHandler::renameSensorData(const int index, const QString new_label)
{
  data_mutex_.lock();

  if (index >= data_.second.size())
  {
    data_mutex_.unlock();
    return false;
  }

  // Search if this label already exists
  QString old_label(data_.second[index].label);
  for (SensorData data : data_.second)
  {
    if (data.label == new_label && old_label != new_label)
    {
      // Renaming "label_0" to "label_0" is NOT an error
      data_mutex_.unlock();
      return false;
    }
  }

  data_.second[index].label = new_label;
  data_mutex_.unlock();
  return true;
}

bool SensorDataHandler::renameSensorDataUnit(const QString label, const QString new_unit)
{
  data_mutex_.lock();

  // Search for label and store it's index
  bool found(false);
  int i;
  for (i = 0; i < data_.second.size(); ++i)
  {
    if (data_.second.at(i).label == label)
    {
      found = true;
      break;
    }
  }

  if (!found)
  {
    data_mutex_.unlock();
    return false;
  }

  data_.second[i].unit = new_unit;
  data_mutex_.unlock();
  return true;
}

bool SensorDataHandler::renameSensorDataUnit(const int index, const QString new_unit)
{
  data_mutex_.lock();

  if (index >= data_.second.size())
  {
    data_mutex_.unlock();
    return false;
  }

  data_.second[index].unit = new_unit;
  data_mutex_.unlock();
  return true;
}

int SensorDataHandler::getSensorDataSize()
{
  return (data_.second.size());
}

int SensorDataHandler::getTimeVectorSize()
{
  return (data_.first.size());
}

const SensorDataHandler::TimeSensorDataVectors& SensorDataHandler::getTimeSensorDataVectors()
{
  return data_;
}

const SensorDataHandler::TimeVector& SensorDataHandler::getTimeVector()
{
  return data_.first;
}

const SensorDataHandler::SensorDataVector& SensorDataHandler::getSensorDataVector()
{
  return data_.second;
}

SensorDataHandler::SensorData const* SensorDataHandler::getSensorData(const QString label)
{
  data_mutex_.lock();
  SensorData const* sensor_data(nullptr);

  // Search for label and store it's index
  for (int i(0); i < data_.second.size(); ++i)
  {
    if (data_.second.at(i).label == label)
    {
      sensor_data = &data_.second.at(i);
      break;
    }
  }

  data_mutex_.unlock();
  return sensor_data;
}

QStringList SensorDataHandler::getSensorDataLabels()
{
  QStringList tmp;
  for (SensorData data : data_.second)
    tmp.append(data.label);

  return tmp;
}

QStringList SensorDataHandler::getSensorDataUnits()
{
  QStringList tmp;
  for (SensorData data : data_.second)
    tmp.append(data.unit);

  return tmp;
}

bool SensorDataHandler::readCSVFile(const QString file_name)
{
  QFile file(file_name);
  if (!file.open(QFile::ReadOnly | QFile::Text))
  {
    QMessageBox msg(QMessageBox::Warning, "Error opening file", "Cannot open " + file_name + " for reading",
                    QMessageBox::Ok);
    msg.exec();
    return false;
  }

  QTextStream stream(&file);
  QString line;
  line = stream.readLine(); // Header line
  QStringList parts = line.split(csv_separator_);

  // We need at least one data (time; data)
  // First column must be time
  if (parts.size() < 2 || !parts[0].contains("time", Qt::CaseInsensitive))
  {
    file.close();
    qDebug() << "readCSVFile: first line does not contain \"time\"";
    return false;
  }

  // Delete last part if it's empty
  if (parts.back().size() == 0)
    parts.removeLast();

  data_mutex_.lock();
  clearTimeAndSensorDataVectors();

  // Add data labels/units
  for (int i(1); i < parts.size(); ++i)
  {
    QString label;
    QString unit;

    label = parts[i];
    if (label.indexOf("(") != -1)
    {
      unsigned id(label.indexOf("("));
      label.truncate(id);

      unit = parts[i];
      unit.remove(0, id);
      unit.remove('(');
      unit.remove(')');
      unit = unit.simplified();
    }
    label = label.simplified();

    if (!addSensorData(label, unit))
    {
      file.close();
      data_mutex_.unlock();
      qDebug() << "readCSVFile: cannot add data";
      return false;
    }
  }

  // Parse data and fill vectors
  while (!line.isNull())
  {
    line = stream.readLine(); // Header line
    QStringList parts = line.split(csv_separator_);
    if (parts.size() <= 1)
      continue;

    // Delete last part if it's empty
    if (parts.back().size() == 0)
      parts.removeLast();

    if (parts.size() != getSensorDataSize() + 1) // Data + time
    {
      file.close();
      qDebug() << "readCSVFile: wrong data count," << parts.size() << "!=" << getSensorDataSize() + 1;
      clearTimeAndSensorDataVectors();
      data_mutex_.unlock();
      return false;
    }

    // Test if decimal separator is dot or comma
    QLocale loc(QLocale::C);
    // Initialize to NaN
    double time_value_secs_msecs(0);
    bool conversion_success(false);
    time_value_secs_msecs = loc.toDouble(parts[0], &conversion_success);

    if (!conversion_success)
    {
      loc = QLocale::French;
      time_value_secs_msecs = loc.toDouble(parts[0], &conversion_success);

      if (!conversion_success)
      {
        qDebug() << "readCSVFile: cannot parse value[0] :" << parts[0];
        return false;
      }
    }

    double integral, fractionnal;
    fractionnal = std::modf(time_value_secs_msecs, &integral);

    unsigned time_value_secs(integral);
    unsigned time_value_msecs(fractionnal * 10e2);

    QDateTime time;
    time.setTime_t(time_value_secs);
    time = time.addMSecs(time_value_msecs);
    //qDebug() << time;

    QVector<double> data;
    for (int i(1); i < parts.size(); ++i)
    {
      bool conversion_success(false);
      double value(loc.toDouble(parts[i], &conversion_success));
      if (!conversion_success)
      {
        qDebug() << "readCSVFile: cannot parse value" << i << "=" << parts[i];
        return false;
      }
      data.push_back(value);
    }

    if (!pushBackTimedSensorDataValues(time, data))
    {
      file.close();
      clearTimeAndSensorDataVectors();
      data_mutex_.unlock();
      qDebug() << "readCSVFile: cannot push data";
      return false;
    }
  }

  file.close();
  data_mutex_.unlock();
  return true;
}

bool SensorDataHandler::writeCSVFile(const QString file_name)
{
  data_mutex_.lock();
  if (data_.first.empty())
  {
    data_mutex_.unlock();
    return true; // There was nothing to write
  }

  QFile file(file_name);
  if (!file.open(QFile::WriteOnly | QFile::Text))
  {
    QMessageBox msg(QMessageBox::Warning, "Error opening file", "Cannot open file " + file_name + " for writing",
                    QMessageBox::Ok);
    data_mutex_.unlock();
    msg.exec();
    return false;
  }

  QTextStream out(&file);

  // Write labels + units: My label (unit)
  out << "time (sec)" + csv_separator_;

  for (SensorData data : data_.second)
  {
    out << data.label;
    if (data.unit.size() != 0) // Only print unit if there is one
      out << " (" + data.unit + ")";
    out << csv_separator_;
  }
  out << endl;

  // Settings for numbers formatting
  QLocale loc(QLocale::C);
  if (comma_decimal_separator_)
    loc = QLocale::French;
  loc.setNumberOptions(QLocale::OmitGroupSeparator); // No thousand separator

  for (int i(0); i < data_.first.size(); ++i)
  {
    // Time
    double time(data_.first.at(i).toMSecsSinceEpoch() / 1000.0);
    out << loc.toString(time, 'f', 3) << csv_separator_; // Specify decimals

    // Data
    for (SensorDataVector::iterator it(data_.second.begin()); it != data_.second.end(); ++it)
      out << loc.toString(it->values.at(i)) << csv_separator_; // FIXME: Do I need a specific format?

    out << endl;
  }

  data_mutex_.unlock();
  return true;
}

bool SensorDataHandler::pushBackTimedSensorDataValues(const QDateTime time, const QVector<double> sensor_data_values)
{
  if (time.isNull())
    return false;
  if (!time.isValid())
    return false;

  // Time must be greater than the last one
  if (getTimeVectorSize() != 0 && time < data_.first.back())
  {
    qDebug() << "pushBackTimedSensorDataValues: time is anterior to previous time stored";
    return false;
  }

  data_mutex_.lock();

  if (getSensorDataSize() == 0)
  {
    qDebug() << "pushBackTimedSensorDataValues: sensor data is empty!";
    return false;
  }

  // Check that number of pushed values matches the number of sensor data
  if (sensor_data_values.size() != getSensorDataSize())
  {
    qDebug() << "pushBackTimedSensorDataValues: wrong number of values in sensor_data_values!"
        << sensor_data_values.size() << "!=" << getSensorDataSize();
    data_mutex_.unlock();
    return false;
  }

  data_.first.push_back(time);
  unsigned i(0);
  for (SensorData& data : data_.second)
    data.values.push_back(sensor_data_values.at(i++));

  data_mutex_.unlock();
  return true;
}
