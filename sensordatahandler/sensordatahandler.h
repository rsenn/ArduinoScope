#ifndef SENSORDATAHANDLER_H
#define SENSORDATAHANDLER_H

#include <QObject>
#include <QStringList>
#include <QDateTime>
#include <QVector>
#include <QPair>
#include <QMutex>

class SensorDataHandler
{
public:

  struct SensorData
  {
    QString label; // Name of the data
    QString unit; // Unit of the data
    QVector<double> values; // Vector of values
  };

  typedef QVector<SensorData> SensorDataVector; // Vector of data
  typedef QVector<QDateTime> TimeVector; // Time
  typedef QPair<TimeVector, SensorDataVector> TimeSensorDataVectors; // Time / vector of data

  SensorDataHandler();

  /**
   * @param label
   * @param unit
   * @return True if successful, false if label already exists otherwise
   * @note The label cannot be an empty string, the unit is allowed to be emtpy
   */
  bool addSensorData(const QString label, const QString unit);

  /**
   * @param label
   * @return True if the label was found and removed, false otherwise
   */
  bool removeSensorData(const QString label);

  /**
   * @param index
   * @return True if the label was found and removed, false otherwise
   */
  bool removeSensorData(const int index);

  /**
   * @param size must be greater or equal than the current SensorDataVector size
   * @return False if size is not greater (or equal) than actual SensorData size, true otherwise
   */
  bool shrinkSensorDataVector(const int size);

  /**
   * Clears the time vector and SensorData vectors
   */
  void clearTimeAndSensorDataVectors();

  /**
   * Clears all data values and the time vector
   */
  void clearTimeVectorAndSensorDataValues();

  /**
   * Rename a label providing it's current name
   * @param old_label
   * @param new_label
   * @return True if successful, false otherwise
   * @note if new_label is already used, returns false!
   */
  bool renameSensorData(const QString old_label, const QString new_label);

  /**
   * Rename a label providing it's index in the vector
   * @param index
   * @param new_label
   * @return True if successful, false otherwise
   */
  bool renameSensorData(const int index, const QString new_label);

  /**
   * Rename a unit providing the data label
   * @param data_label
   * @param new_unit
   * @return True if successful, false otherwise
   */
  bool renameSensorDataUnit(const QString data_label, const QString new_unit);

  /**
   * Rename a unit providing the data index in the vector
   * @param index
   * @param new_unit
   * @return True if successful, false otherwise
   */
  bool renameSensorDataUnit(const int index, const QString new_unit);

  /**
   * @returns the number of sensor data
   */
  int getSensorDataSize();

  /**
   * @returns the number of time values stored
   * @note this is always equal to @c SensorData values size
   */
  int getTimeVectorSize();

  /**
   * @returns a const reference to the time and data vectors
   */
  const TimeSensorDataVectors& getTimeSensorDataVectors();

  /**
   * @returns a const reference to the time vector
   */
  const TimeVector& getTimeVector();

  /**
   * @returns a const reference to the sensor data vector
   */
  const SensorDataVector& getSensorDataVector();

  /**
   * Get the sensor data structure corresponding to the label
   * @param label
   * @param sensor_data
   */
  SensorData const* getSensorData(const QString label);

  /**
   * @returns a list of strings containing the sensor data labels
   */
  QStringList getSensorDataLabels();

  /**
   * @return a list of strings containing the sensor data units
   */
  QStringList getSensorDataUnits();

  /**
   * Read CSV textual file on the disk containing all the data, values are ";" separated
   * @param file_name
   * @return True if successful, false otherwise
   * @warning All existing data is cleared before reading
   */
  bool readCSVFile(const QString file_name);

  /**
   * Write a CSV textual file on the disk containing all the data, values are ";" separated
   * @param file_name
   * @return True if successful, false otherwise
   */
  bool writeCSVFile(const QString file_name);

public slots:
  /**
   * Sensor data values must be ordered in the same way they were added
   * @param time the time corresponding to the data values
   * @param sensor_data_values the values
   * @return True if successful, false otherwise
   * @note sensor_data_values must match getSensorDataSize return value
   * @note time must be greater than the last time value pushed
   */
  bool pushBackTimedSensorDataValues(const QDateTime time, const QVector<double> sensor_data_values);

  /**
   * Controls whether the output file should use a dot separator (default) or a comma decimal separator.
   */
  bool comma_decimal_separator_;

private:
  QMutex data_mutex_; // Recursive, see constructor
  TimeSensorDataVectors data_;
  const QString csv_separator_; // How values are separated
};

#endif // SENSORDATAHANDLER_H
